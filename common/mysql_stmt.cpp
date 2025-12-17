#include "mysql_stmt.h"

#include "common/eqemu_logsys.h"
#include "common/mutex.h"
#include "common/timer.h"

#include <charconv>

namespace mysql
{

void PreparedStmt::StmtDeleter::operator()(MYSQL_STMT* stmt) noexcept
{
	// The connection must be locked when closing the stmt to avoid mysql errors
	// in case another thread tries to use it during the close. If the mutex is
	// changed to one that throws then exceptions need to be caught here.
	LockMutex lock(mutex);
	mysql_stmt_close(stmt);
}

PreparedStmt::PreparedStmt(MYSQL& mysql, std::string query, Mutex* mutex, StmtOptions opts)
	: m_stmt(mysql_stmt_init(&mysql), { mutex }), m_query(std::move(query)), m_mutex(mutex), m_options(opts)
{
	LockMutex lock(m_mutex);
	if (mysql_stmt_prepare(m_stmt.get(), m_query.c_str(), static_cast<unsigned long>(m_query.size())) != 0)
	{
		ThrowError(fmt::format("Prepare error: {}", GetStmtError()));
	}

	m_params.resize(mysql_stmt_param_count(m_stmt.get()));
	m_inputs.resize(m_params.size());
}

void PreparedStmt::ThrowError(const std::string& error)
{
	LogMySQLError("{}", error);
	throw std::runtime_error(error);
}

std::string PreparedStmt::GetStmtError()
{
	auto err = mysql_stmt_errno(m_stmt.get());
	auto str = mysql_stmt_error(m_stmt.get());
	return fmt::format("({}) [{}] for query [{}]", err, str, m_query);
}

template <typename T>
void PreparedStmt::BindInput(size_t index, T value)
{
	if (index >= m_inputs.size())
	{
		ThrowError(fmt::format("Cannot bind input, index {} out of range", index));
	}

	impl::Bind& arg  = m_inputs[index];
	arg.is_null      = std::is_same_v<T, std::nullptr_t>;

	MYSQL_BIND& bind = m_params[index];
	bind.is_unsigned = std::is_unsigned_v<T>;
	bind.is_null     = &arg.is_null;
	bind.length      = &arg.length;

	auto old_type = bind.buffer_type;

	if constexpr (std::is_arithmetic_v<T>)
	{
		if (arg.buffer.size() < sizeof(T))
		{
			arg.buffer.resize(std::max(sizeof(T), sizeof(int64_t)));
			bind.buffer = arg.buffer.data();
			m_need_bind = true;
		}
		memcpy(arg.buffer.data(), &value, sizeof(T));
	}

	if constexpr (std::is_same_v<T, int8_t> || std::is_same_v<T, uint8_t> || std::is_same_v<T, bool>)
	{
		bind.buffer_type = MYSQL_TYPE_TINY;
	}
	else if constexpr (std::is_same_v<T, int16_t> || std::is_same_v<T, uint16_t>)
	{
		bind.buffer_type = MYSQL_TYPE_SHORT;
	}
	else if constexpr (std::is_same_v<T, int32_t> || std::is_same_v<T, uint32_t>)
	{
		bind.buffer_type = MYSQL_TYPE_LONG;
	}
	else if constexpr (std::is_same_v<T, int64_t> || std::is_same_v<T, uint64_t>)
	{
		bind.buffer_type = MYSQL_TYPE_LONGLONG;
	}
	else if constexpr (std::is_same_v<T, float>)
	{
		bind.buffer_type = MYSQL_TYPE_FLOAT;
	}
	else if constexpr (std::is_same_v<T, double>)
	{
		bind.buffer_type = MYSQL_TYPE_DOUBLE;
	}
	else if constexpr (std::is_same_v<T, std::string_view>)
	{
		bind.buffer_type = MYSQL_TYPE_STRING;
		if (arg.buffer.empty() || arg.buffer.size() < value.size())
		{
			arg.buffer.resize(static_cast<size_t>((value.size() + 1) * 1.5));
			bind.buffer = arg.buffer.data();
			bind.buffer_length = static_cast<unsigned long>(arg.buffer.size());
			m_need_bind = true;
		}
		std::copy(value.begin(), value.end(), arg.buffer.begin());
		arg.length = static_cast<unsigned long>(value.size());
	}
	else if constexpr (!std::is_same_v<T, std::nullptr_t>)
	{
		static_assert(false_v<T>, "Cannot bind unsupported type");
	}

	if (old_type != bind.buffer_type)
	{
		m_need_bind = true;
	}
}

void PreparedStmt::BindInput(size_t index, const char* str)
{
	BindInput(index, std::string_view(str));
}

void PreparedStmt::BindInput(size_t index, const std::string& str)
{
	BindInput(index, std::string_view(str));
}

StmtResult PreparedStmt::Execute()
{
	CheckArgs(0);
	return DoExecute();
}

StmtResult PreparedStmt::Execute(const std::vector<param_t>& args)
{
	CheckArgs(args.size());
	for (size_t i = 0; i < args.size(); ++i)
	{
		std::visit([&](const auto& arg) { BindInput(i, arg); }, args[i]);
	}
	return DoExecute();
}

template <typename T>
StmtResult PreparedStmt::Execute(const std::vector<T>& args)
{
	CheckArgs(args.size());
	for (size_t i = 0; i < args.size(); ++i)
	{
		BindInput(i, args[i]);
	}
	return DoExecute();
}

void PreparedStmt::CheckArgs(size_t argc)
{
	if (argc != m_params.size())
	{
		ThrowError(fmt::format("Bad arg count (got {}, expected {}) for [{}]", argc, m_params.size(), m_query));
	}
}

StmtResult PreparedStmt::DoExecute()
{
	BenchTimer timer;
	LockMutex lock(m_mutex);

	if (m_need_bind && mysql_stmt_bind_param(m_stmt.get(), m_params.data()) != 0)
	{
		ThrowError(fmt::format("Bind param error: {}", GetStmtError()));
	}

	m_need_bind = false;

	if (mysql_stmt_execute(m_stmt.get()) != 0)
	{
		ThrowError(fmt::format("Execute error: {}", GetStmtError()));
	}

	my_bool attr = m_options.use_max_length;
	mysql_stmt_attr_set(m_stmt.get(), STMT_ATTR_UPDATE_MAX_LENGTH, &attr);

	if (m_options.buffer_results && mysql_stmt_store_result(m_stmt.get()) != 0)
	{
		ThrowError(fmt::format("Store result error: {}", GetStmtError()));
	}

	// Result buffers are bound on first execute and re-used if needed
	if (m_results.empty())
	{
		BindResults();
	}

	StmtResult res(m_stmt.get(), m_results.size());

	if (m_results.empty())
	{
		LogMySQLQuery("{} -- ({} row(s) affected) ({:.6f}s)", m_query, res.RowsAffected(), timer.elapsed());
	}
	else
	{
		LogMySQLQuery("{} -- ({} row(s) returned) ({:.6f}s)", m_query, res.RowCount(), timer.elapsed());
	}

	return res;
}

void PreparedStmt::BindResults()
{
	MYSQL_RES* res = mysql_stmt_result_metadata(m_stmt.get());
	if (!res)
	{
		return; // did not produce a result set
	}

	MYSQL_FIELD* fields = mysql_fetch_fields(res);
	m_columns.resize(mysql_num_fields(res));
	m_results.resize(m_columns.size());

	for (int i = 0; i < static_cast<int>(m_columns.size()); ++i)
	{
		impl::BindColumn& col = m_columns[i].m_col;
		MYSQL_BIND& bind = m_results[i];

		col.index          = i;
		col.name           = fields[i].name;
		col.buffer_type    = fields[i].type;
		col.is_unsigned    = (fields[i].flags & UNSIGNED_FLAG) != 0;
		col.buffer.resize(GetResultBufferSize(fields[i]));

		bind.buffer_type   = col.buffer_type;
		bind.buffer        = col.buffer.data();
		bind.buffer_length = static_cast<unsigned long>(col.buffer.size());
		bind.is_unsigned   = col.is_unsigned;
		bind.is_null       = &col.is_null;
		bind.length        = &col.length;
		bind.error         = &col.error;
	}

	mysql_free_result(res);

	if (!m_results.empty() && mysql_stmt_bind_result(m_stmt.get(), m_results.data()) != 0)
	{
		ThrowError(fmt::format("Bind result error: {}", GetStmtError()));
	}
}

int PreparedStmt::GetResultBufferSize(const MYSQL_FIELD& field) const
{
	switch (field.type)
	{
	case MYSQL_TYPE_TINY:
		return sizeof(int8_t);
	case MYSQL_TYPE_SHORT:
		return sizeof(int16_t);
	case MYSQL_TYPE_INT24:
	case MYSQL_TYPE_LONG:
		return sizeof(int32_t);
	case MYSQL_TYPE_LONGLONG:
		return sizeof(int64_t);
	case MYSQL_TYPE_FLOAT:
		return sizeof(float);
	case MYSQL_TYPE_DOUBLE:
		return sizeof(double);
	case MYSQL_TYPE_TIME:
	case MYSQL_TYPE_DATE:
	case MYSQL_TYPE_DATETIME:
	case MYSQL_TYPE_TIMESTAMP:
		return sizeof(MYSQL_TIME);
	default: // if max_length is unavailable for strings buffers are resized on fetch
		return field.max_length + 1; // ensure valid buffer created
	}
}

StmtRow PreparedStmt::Fetch()
{
	StmtRow row;
	if (!m_columns.empty())
	{
		int rc = mysql_stmt_fetch(m_stmt.get());
		if (rc == 1)
		{
			ThrowError(fmt::format("Fetch error: {}", GetStmtError()));
		}

		if (rc != MYSQL_NO_DATA)
		{
			if (rc == MYSQL_DATA_TRUNCATED)
			{
				FetchTruncated();
			}
			row = StmtRow(m_columns);
		}
	}
	return row;
}

void PreparedStmt::FetchTruncated()
{
	for (int i = 0; i < static_cast<int>(m_columns.size()); ++i)
	{
		impl::BindColumn& col = m_columns[i].m_col;
		if (col.error)
		{
			MYSQL_BIND& bind = m_results[i];
			col.buffer.resize(static_cast<size_t>(col.length * 1.5));
			bind.buffer = col.buffer.data();
			bind.buffer_length = static_cast<unsigned long>(col.buffer.size());

			mysql_stmt_fetch_column(m_stmt.get(), &bind, i, 0);
		}
	}

	if (mysql_stmt_bind_result(m_stmt.get(), m_results.data()) != 0)
	{
		ThrowError(fmt::format("Fetch rebind result error: {}", GetStmtError()));
	}
}

// ---------------------------------------------------------------------------

StmtResult::StmtResult(MYSQL_STMT* stmt, size_t columns)
{
	m_num_cols  = static_cast<int>(columns);
	m_num_rows  = mysql_stmt_num_rows(stmt); // requires buffered results
	m_affected  = mysql_stmt_affected_rows(stmt);
	m_insert_id = mysql_stmt_insert_id(stmt);
}

// ---------------------------------------------------------------------------

const StmtColumn* StmtRow::GetColumn(size_t index) const
{
	return index < m_columns.size() ? &m_columns[index] : nullptr;
}

const StmtColumn* StmtRow::GetColumn(std::string_view name) const
{
	auto it = std::ranges::find_if(m_columns,
		[name](const StmtColumn& col) { return col.Name() == name; });

	return it != m_columns.end() ? &(*it) : nullptr;
}

std::optional<std::string> StmtRow::operator[](size_t index) const
{
	return GetStr(index);
}

std::optional<std::string> StmtRow::operator[](std::string_view name) const
{
	return GetStr(name);
}

std::optional<std::string> StmtRow::GetStr(size_t index) const
{
	const StmtColumn* col = GetColumn(index);
	return col ? col->GetStr() : std::nullopt;
}

std::optional<std::string> StmtRow::GetStr(std::string_view name) const
{
	const StmtColumn* col = GetColumn(name);
	return col ? col->GetStr() : std::nullopt;
}

template <typename T> requires std::is_arithmetic_v<T>
std::optional<T> StmtRow::Get(size_t index) const
{
	const StmtColumn* col = GetColumn(index);
	return col ? col->Get<T>() : std::nullopt;
}

template <typename T> requires std::is_arithmetic_v<T>
std::optional<T> StmtRow::Get(std::string_view name) const
{
	const StmtColumn* col = GetColumn(name);
	return col ? col->Get<T>() : std::nullopt;
}

// ---------------------------------------------------------------------------

static time_t MakeTime(const MYSQL_TIME& mt)
{
	// buffer mt given in mysql session time zone (assumes local)
	std::tm tm{};
	tm.tm_year  = mt.year - 1900;
	tm.tm_mon   = mt.month - 1;
	tm.tm_mday  = mt.day;
	tm.tm_hour  = mt.hour;
	tm.tm_min   = mt.minute;
	tm.tm_sec   = mt.second;
	tm.tm_isdst = -1;
	return std::mktime(&tm);
}

static int MakeSeconds(const MYSQL_TIME& mt)
{
	return (mt.neg ? -1 : 1) * static_cast<int>(mt.hour * 3600 + mt.minute * 60 + mt.second);
}

static uint64_t MakeBits(std::span<const uint8_t> data)
{
	// byte stream for bits is in big endian
	uint64_t bits = 0;
	for (size_t i = 0; i < data.size() && i < sizeof(uint64_t); ++i)
	{
		bits |= static_cast<uint64_t>(data[data.size() - i - 1] & 0xff) << (i * 8);
	}
	return bits;
}

template <typename T>
concept has_from_chars = requires (const char* first, const char* last, T value)
{
	std::from_chars(first, last, value);
};

template <typename T>
static T FromString(std::string_view sv)
{
	if constexpr (std::is_same_v<T, bool>)
	{
		// return false for empty (zero-length) strings
		return !sv.empty();
	}
	else if constexpr (std::is_same_v<T, float> && !has_from_chars<T>)
	{
		return std::strtof(std::string(sv).c_str(), nullptr);
	}
	else if constexpr (std::is_same_v<T, double> && !has_from_chars<T>)
	{
		return std::strtod(std::string(sv).c_str(), nullptr);
	}
	else
	{
		// non numbers return a zero initialized T (could return nullopt instead)
		T value = {};
		std::from_chars(sv.data(), sv.data() + sv.size(), value);
		return value;
	}
}

static std::string FormatTime(enum_field_types type, const MYSQL_TIME& mt)
{
	switch (type)
	{
	case MYSQL_TYPE_TIME: // hhh:mm:ss '-838:59:59' to '838:59:59'
		return fmt::format("{}{:02d}:{:02d}:{:02d}", mt.neg ? "-" : "", mt.hour, mt.minute, mt.second);
	case MYSQL_TYPE_DATE: // YYYY-MM-DD '1000-01-01' to '9999-12-31'
		return fmt::format("{}-{:02d}-{:02d}", mt.year, mt.month, mt.day);
	case MYSQL_TYPE_DATETIME:  // YYYY-MM-DD hh:mm:ss '1000-01-01 00:00:00' to '9999-12-31 23:59:59'
	case MYSQL_TYPE_TIMESTAMP: // YYYY-MM-DD hh:mm:ss '1970-01-01 00:00:01' UTC to '2038-01-19 03:14:07' UTC
		return fmt::format("{}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}", mt.year, mt.month, mt.day, mt.hour, mt.minute, mt.second);
	default:
		return std::string();
	}
}

std::optional<std::string_view> StmtColumn::GetStrView() const
{
	if (m_col.is_null)
	{
		return std::nullopt;
	}

	switch (m_col.buffer_type)
	{
	case MYSQL_TYPE_NEWDECIMAL:
	case MYSQL_TYPE_TINY_BLOB:
	case MYSQL_TYPE_MEDIUM_BLOB:
	case MYSQL_TYPE_LONG_BLOB:
	case MYSQL_TYPE_BLOB:
	case MYSQL_TYPE_VAR_STRING:
	case MYSQL_TYPE_STRING:
		return std::make_optional<std::string_view>(reinterpret_cast<const char*>(m_col.buffer.data()), m_col.length);
	default:
		return std::nullopt;
	}
}

std::optional<std::string> StmtColumn::GetStr() const
{
	if (m_col.is_null)
	{
		return std::nullopt;
	}

	switch (m_col.buffer_type)
	{
	case MYSQL_TYPE_TINY:
		return m_col.is_unsigned ? fmt::format_int(BitCast<uint8_t>()).c_str() : fmt::format_int(BitCast<int8_t>()).c_str();
	case MYSQL_TYPE_SHORT:
		return m_col.is_unsigned ? fmt::format_int(BitCast<uint16_t>()).c_str() : fmt::format_int(BitCast<int16_t>()).c_str();
	case MYSQL_TYPE_INT24:
	case MYSQL_TYPE_LONG:
		return m_col.is_unsigned ? fmt::format_int(BitCast<uint32_t>()).c_str() : fmt::format_int(BitCast<int32_t>()).c_str();
	case MYSQL_TYPE_LONGLONG:
		return m_col.is_unsigned ? fmt::format_int(BitCast<uint64_t>()).c_str() : fmt::format_int(BitCast<int64_t>()).c_str();
	case MYSQL_TYPE_FLOAT:
		return fmt::format("{}", BitCast<float>());
	case MYSQL_TYPE_DOUBLE:
		return fmt::format("{}", BitCast<double>());
	case MYSQL_TYPE_TIME:
	case MYSQL_TYPE_DATE:
	case MYSQL_TYPE_DATETIME:
	case MYSQL_TYPE_TIMESTAMP:
		return FormatTime(m_col.buffer_type, BitCast<MYSQL_TIME>());
	case MYSQL_TYPE_BIT:
		return fmt::format_int(*Get<uint64_t>()).c_str();
	case MYSQL_TYPE_NEWDECIMAL:
	case MYSQL_TYPE_TINY_BLOB:
	case MYSQL_TYPE_MEDIUM_BLOB:
	case MYSQL_TYPE_LONG_BLOB:
	case MYSQL_TYPE_BLOB:
	case MYSQL_TYPE_VAR_STRING:
	case MYSQL_TYPE_STRING:
		return std::make_optional<std::string>(reinterpret_cast<const char*>(m_col.buffer.data()), m_col.length);
	default:
		return std::nullopt;
	}
}

template <typename T> requires std::is_arithmetic_v<T>
std::optional<T> StmtColumn::Get() const
{
	if (m_col.is_null)
	{
		return std::nullopt;
	}

	switch (m_col.buffer_type)
	{
	case MYSQL_TYPE_TINY:
		return m_col.is_unsigned ? static_cast<T>(BitCast<uint8_t>()) : static_cast<T>(BitCast<int8_t>());
	case MYSQL_TYPE_SHORT:
		return m_col.is_unsigned ? static_cast<T>(BitCast<uint16_t>()) : static_cast<T>(BitCast<int16_t>());
	case MYSQL_TYPE_INT24:
	case MYSQL_TYPE_LONG:
		return m_col.is_unsigned ? static_cast<T>(BitCast<uint32_t>()) : static_cast<T>(BitCast<int32_t>());
	case MYSQL_TYPE_LONGLONG:
		return m_col.is_unsigned ? static_cast<T>(BitCast<uint64_t>()) : static_cast<T>(BitCast<int64_t>());
	case MYSQL_TYPE_FLOAT:
		return static_cast<T>(BitCast<float>());
	case MYSQL_TYPE_DOUBLE:
		return static_cast<T>(BitCast<double>());
	case MYSQL_TYPE_TIME: // return as total seconds
		return static_cast<T>(MakeSeconds(BitCast<MYSQL_TIME>()));
	case MYSQL_TYPE_DATE:
	case MYSQL_TYPE_DATETIME:
	case MYSQL_TYPE_TIMESTAMP: // return as epoch timestamp
		return static_cast<T>(MakeTime(BitCast<MYSQL_TIME>()));
	case MYSQL_TYPE_BIT:
		return static_cast<T>(MakeBits({ m_col.buffer.data(), m_col.length }));
	case MYSQL_TYPE_NEWDECIMAL:
	case MYSQL_TYPE_TINY_BLOB:
	case MYSQL_TYPE_MEDIUM_BLOB:
	case MYSQL_TYPE_LONG_BLOB:
	case MYSQL_TYPE_BLOB:
	case MYSQL_TYPE_VAR_STRING:
	case MYSQL_TYPE_STRING:
		return FromString<T>({ reinterpret_cast<const char*>(m_col.buffer.data()), m_col.length });
	default:
		return std::nullopt;
	}
}

// ---------------------------------------------------------------------------

// explicit template instantiations for supported types
template void PreparedStmt::BindInput(size_t, std::string_view);
template void PreparedStmt::BindInput(size_t, std::nullptr_t);
template StmtResult PreparedStmt::Execute(const std::vector<std::string_view>&);
template StmtResult PreparedStmt::Execute(const std::vector<std::string>&);
template StmtResult PreparedStmt::Execute(const std::vector<const char*>&);

#define INSTANTIATE(T) \
	template void PreparedStmt::BindInput(size_t, T); \
	template StmtResult PreparedStmt::Execute(const std::vector<T>&); \
	template std::optional<T> StmtRow::Get(size_t) const; \
	template std::optional<T> StmtRow::Get(std::string_view) const; \
	template std::optional<T> StmtColumn::Get() const;

INSTANTIATE(bool);
INSTANTIATE(int8_t);
INSTANTIATE(uint8_t);
INSTANTIATE(int16_t);
INSTANTIATE(uint16_t);
INSTANTIATE(int32_t);
INSTANTIATE(uint32_t);
INSTANTIATE(int64_t);
INSTANTIATE(uint64_t);
INSTANTIATE(float);
INSTANTIATE(double);

} // namespace mysql
