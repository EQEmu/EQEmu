#pragma once

#include "mysql.h"
#include <cassert>
#include <cstring>
#include <memory>
#include <optional>
#include <span>
#include <string_view>
#include <string>
#include <variant>
#include <vector>

class Mutex;

namespace mysql
{

// support MySQL 8.0.1+ API which removed the my_bool type
#if !defined(MARIADB_VERSION_ID) && MYSQL_VERSION_ID >= 80001
using my_bool = bool;
#endif

template <typename>
inline constexpr bool false_v = false;

namespace impl
{

struct Bind
{
	std::vector<uint8_t> buffer;
	unsigned long length = 0;
	my_bool is_null = false;
	my_bool error = false;
};

struct BindColumn : Bind
{
	int index = 0;
	std::string name;
	bool is_unsigned = false;
	enum_field_types buffer_type = {};
};

} // namespace impl

// ---------------------------------------------------------------------------

struct StmtOptions
{
	// Enable buffering (storing) entire result set after executing a statement
	bool buffer_results = true;

	// Enable MySQL to update max_length of fields in execute result set (requires buffering)
	bool use_max_length = true;
};

// ---------------------------------------------------------------------------

// Holds ownership of bound column value buffer
class StmtColumn
{
public:
	int Index() const { return m_col.index; }
	bool IsNull() const { return m_col.is_null; }
	bool IsUnsigned() const { return m_col.is_unsigned; }
	enum_field_types Type() const { return m_col.buffer_type; }
	const std::string& Name() const { return m_col.name; }

	// Get view of column value buffer
	std::span<const uint8_t> GetBuf() const { return { m_col.buffer.data(), m_col.length }; }

	// Get view of column string value. Returns nullopt if value is NULL or not a string
	std::optional<std::string_view> GetStrView() const;

	// Get column value as string. Returns nullopt if value is NULL or field type unsupported
	std::optional<std::string> GetStr() const;

	// Get column value as numeric T. Returns nullopt if value NULL or field type unsupported
	template <typename T> requires std::is_arithmetic_v<T>
	std::optional<T> Get() const;

private:
	// uses memcpy for type punning buffer data to avoid UB with strict aliasing
	template <typename T>
	T BitCast() const
	{
		T val;
		assert(sizeof(T) == m_col.length);
		memcpy(&val, m_col.buffer.data(), sizeof(T));
		return val;
	}

	friend class PreparedStmt; // access to allocate and bind buffers
	friend class StmtResult; // access to resize truncated buffers
	impl::BindColumn m_col;
};

// ---------------------------------------------------------------------------

// Provides a non-owning view of PreparedStmt column value buffers
// Evaluates false if it does not contain a valid row
class StmtRow
{
public:
	StmtRow() = default;
	StmtRow(std::span<const StmtColumn> columns) : m_columns(columns) {};

	explicit operator bool() const noexcept { return !m_columns.empty(); }

	int ColumnCount() const { return static_cast<int>(m_columns.size()); }
	const StmtColumn* GetColumn(size_t index) const;
	const StmtColumn* GetColumn(std::string_view name) const;

	// Get specified column value as string
	// Returns nullopt if column invalid, value is NULL, or field type unsupported
	std::optional<std::string> operator[](size_t index) const;
	std::optional<std::string> operator[](std::string_view name) const;
	std::optional<std::string> GetStr(size_t index) const;
	std::optional<std::string> GetStr(std::string_view name) const;

	// Get specified column value as numeric T
	// Returns nullopt if column invalid, value is NULL, or field type unsupported
	template <typename T> requires std::is_arithmetic_v<T>
	std::optional<T> Get(size_t index) const;

	template <typename T> requires std::is_arithmetic_v<T>
	std::optional<T> Get(std::string_view name) const;

	auto begin() const { return m_columns.begin(); }
	auto end()   const { return m_columns.end(); }

private:
	std::span<const StmtColumn> m_columns;
};

// ---------------------------------------------------------------------------

// Result meta data for an executed prepared statement
class StmtResult
{
public:
	StmtResult() = default;
	StmtResult(MYSQL_STMT* stmt, size_t columns);

	int ColumnCount() const { return m_num_cols; }
	uint64_t RowCount() const { return m_num_rows; }
	uint64_t RowsAffected() const { return m_affected; }
	uint64_t LastInsertID() const { return m_insert_id; }

private:
	int m_num_cols = 0;
	uint64_t m_num_rows = 0;
	uint64_t m_affected = 0;
	uint64_t m_insert_id = 0;
};

// ---------------------------------------------------------------------------

class PreparedStmt
{
public:
	// Supported argument types for execute
	using param_t = std::variant<int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t,
		int64_t, uint64_t, float, double, bool, std::string_view, std::nullptr_t>;

	PreparedStmt() = delete;
	PreparedStmt(MYSQL& mysql, std::string query, Mutex* mutex, StmtOptions opts = {});

	const std::string& GetQuery() const { return m_query; }
	StmtOptions GetOptions() const { return m_options; }
	void SetOptions(StmtOptions options) { m_options = options; }
	void FreeResult() { mysql_stmt_free_result(m_stmt.get()); }

	// Execute the prepared statement with specified arguments
	// Throws exception on error
	template <typename T>
	StmtResult Execute(const std::vector<T>& args);
	StmtResult Execute(const std::vector<param_t>& args);
	StmtResult Execute();

	// Fetch the next row into column buffers (overwrites previous row values)
	// Return value evaluates false if no more rows to fetch
	// Throws exception on error
	StmtRow Fetch();

private:
	void CheckArgs(size_t argc);
	StmtResult DoExecute();
	void BindResults();
	void FetchTruncated();
	int GetResultBufferSize(const MYSQL_FIELD& field) const;
	void ThrowError(const std::string& error);
	std::string GetStmtError();

	// bind an input value to a query parameter by index
	template <typename T>
	void BindInput(size_t index, T value);
	void BindInput(size_t index, const char* str);
	void BindInput(size_t index, const std::string& str);

	struct StmtDeleter
	{
		Mutex* mutex = nullptr;
		void operator()(MYSQL_STMT* stmt) noexcept;
	};

private:
	std::unique_ptr<MYSQL_STMT, StmtDeleter> m_stmt;
	std::vector<MYSQL_BIND> m_params;  // input binds
	std::vector<MYSQL_BIND> m_results; // result binds
	std::vector<impl::Bind> m_inputs;  // execute buffers (addresses bound)
	std::vector<StmtColumn> m_columns; // fetch buffers (addresses bound)
	std::string m_query;
	StmtOptions m_options = {};
	bool m_need_bind = true;
	Mutex* m_mutex = nullptr; // connection mutex
};

} // namespace mysql
