/*
embperl.cpp
---------------
wraps a perl interpreter for use in eqemu
Eglin
*/

#ifndef EMBPERL_CPP
#define EMBPERL_CPP

#ifdef EMBPERL

#include "../common/global_define.h"
#include "../common/eqemu_logsys.h"
#include <vector>
#include "embperl.h"
#include "../common/features.h"
#include "../common/process/process.h"
#include "../common/file.h"
#include "../common/timer.h"

#ifndef GvCV_set
#define GvCV_set(gv,cv)   (GvCV(gv) = (cv))
#endif

#ifdef EMBPERL_IO_CAPTURE
XS(XS_EQEmuIO_PRINT);
#endif //EMBPERL_IO_CAPTURE

const char* argv_eqemu[] = {
	"",
#ifdef EMBPERL_IO_CAPTURE
	"-w", "-W",
#endif
	"-e", "0;", nullptr
};

#ifdef EMBPERL_IO_CAPTURE
int argc = 5;
#else
int argc = 3;
#endif
//so embedded scripts can use xs extensions (ala 'use socket;')
EXTERN_C void boot_DynaLoader(pTHX_ CV* cv);
EXTERN_C void xs_init(pTHX)
{
	char file[256];
	strncpy(file, __FILE__, 256);
	file[255] = '\0';

	char buf[128];    //shouldent have any function names longer than this.

	//add the strcpy stuff to get rid of const warnings....

	newXS(strcpy(buf, "DynaLoader::boot_DynaLoader"), boot_DynaLoader, file);
#ifdef EMBPERL_IO_CAPTURE
	newXS(strcpy(buf, "EQEmuIO::PRINT"), XS_EQEmuIO_PRINT, file);
#endif
}

Embperl::Embperl()
{
	char** argv = (char**) argv_eqemu;
	char** env  = { nullptr };
	PERL_SYS_INIT3(&argc, &argv, &env);
	DoInit();
}

void Embperl::DoInit()
{
	char** argv = (char**) argv_eqemu;
	my_perl = perl_alloc();
	//setup perl...
	if (!my_perl) {
		throw "Failed to init Perl (perl_alloc)";
	}
	PERL_SET_CONTEXT(my_perl);
	PL_curinterp = (PerlInterpreter*)(my_perl);
	PL_perl_destruct_level = 1;
	perl_construct(my_perl);
	perl_parse(my_perl, xs_init, argc, argv, nullptr);
	perl_run(my_perl);
	
	//a little routine we use a lot.
	eval_pv("sub my_eval { eval $_[0];}", TRUE);    //dies on error
	
	//ruin the perl exit and command:
	eval_pv("sub my_exit {}", TRUE);
	eval_pv("sub my_sleep {}", TRUE);
	if (gv_stashpv("CORE::GLOBAL", FALSE)) {
		GV* exitgp = gv_fetchpv("CORE::GLOBAL::exit", TRUE, SVt_PVCV);
		GvCV_set(exitgp, perl_get_cv("my_exit", TRUE));    //dies on error
		GvIMPORTED_CV_on(exitgp);
		GV* sleepgp = gv_fetchpv("CORE::GLOBAL::sleep", TRUE, SVt_PVCV);
		GvCV_set(sleepgp, perl_get_cv("my_sleep", TRUE));    //dies on error
		GvIMPORTED_CV_on(sleepgp);
	}
	
	//declare our file eval routine.
	try {
		init_eval_file();
	}
	catch (std::string& e) {
		//remember... lasterr() is no good if we crap out here, in construction
		LogQuests("Perl Error [{}]", e);
		throw "failed to install eval_file hook";
	}

#ifdef EMBPERL_IO_CAPTURE
	LogQuests("Tying perl output to eqemu logs");
	//make a tieable class to capture IO and pass it into EQEMuLog
	eval_pv(
		"package EQEmuIO; "
		"sub TIEHANDLE { my $me = bless {}, $_[0]; $me->PRINT('Creating '. $me); return($me); } "
		"sub WRITE {  } "
		"sub PRINTF { my $me = shift; my $fmt = shift; $me->PRINT(sprintf($fmt, @_)); } "
		"sub CLOSE { my $me = shift; $me->PRINT('Closing '.$me); } "
		"sub DESTROY { my $me = shift; $me->PRINT('Destroying '.$me); } "
		"package quest;"
		"	if(tied *STDOUT) { untie(*STDOUT); }"
		"	if(tied *STDERR) { untie(*STDERR); }"
		"	tie *STDOUT, 'EQEmuIO';"
		"	tie *STDERR, 'EQEmuIO';", FALSE);
#endif //EMBPERL_IO_CAPTURE

#ifdef EMBPERL_PLUGIN
	eval_pv(
		"package plugin; ", FALSE
	);

	LogQuests("Loading perlemb plugins");
	try {
		std::string perl_command;
		perl_command = "main::eval_file('plugin', '" + Config->PluginPlFile + "');";
		eval_pv(perl_command.c_str(), FALSE);
	}
	catch (std::string& e) {
		LogQuests("Warning [{}]: [{}]", Config->PluginPlFile, e);
	}

	for (auto & dir : PathManager::Instance()->GetPluginPaths()) {
		try {
			//should probably read the directory in c, instead, so that
			//I can echo filenames as I do it, but c'mon... I'm lazy and this 1 line reads in all the plugins
			const std::string& perl_command = (
				"if(opendir(D,'" +
				dir +
				"')) { "
				"	my @d = readdir(D);"
				"	closedir(D);"
				"	foreach(@d){ "
				"		main::eval_file('plugin','" +
				dir +
				"/'.$_)if/\\.pl$/;"
				"	}"
			"}");
			eval_pv(perl_command.c_str(), FALSE);
		}
		catch (std::string& e) {
			LogQuests("Warning [{}]", e);
		}
	}
#endif //EMBPERL_PLUGIN
}

Embperl::~Embperl()
{
#ifdef EMBPERL_IO_CAPTURE
	eval_pv(
		"package quest;"
		"	if(tied *STDOUT) { untie(*STDOUT); }"
		"	if(tied *STDERR) { untie(*STDERR); }", FALSE);
#endif
	PL_perl_destruct_level = 1;
	perl_destruct(my_perl);
	perl_free(my_perl);
	PERL_SYS_TERM();
	my_perl = NULL;
}

void Embperl::Reinit()
{
	PERL_SET_CONTEXT(my_perl);
	PL_curinterp = (PerlInterpreter*)(my_perl);
	PL_perl_destruct_level = 1;
	perl_destruct(my_perl);
	perl_free(my_perl);
	my_perl = NULL;
	DoInit();
}

void Embperl::init_eval_file(void)
{
	eval_pv(
		"our %Cache;"
		"no warnings 'all';"
		"use Symbol qw(delete_package);"
		"sub eval_file {"
		"my($package, $filename) = @_;"
		"$filename=~s/\'//g;"
		"if(! -r $filename) { print \"Unable to read perl file '$filename'\\n\"; return; }"
		"my $mtime = -M $filename;"
		"if(defined $Cache{$package}{mtime}&&$Cache{$package}{mtime} <= $mtime && !($package eq 'plugin')){"
		"	return;"
		"} else {"
		// we 'my' $filename,$mtime,$package,$sub to prevent them from changing our state up here.
		"	eval(\"package $package; my(\\$filename,\\$mtime,\\$package,\\$sub); \\$isloaded = 1; require './$filename'; \");"
		" print $@ if $@;"
		"}"
		"}", FALSE);
}

int Embperl::eval_file(const char* package_name, const char* filename)
{
	std::vector<std::string> args;
	args.push_back(package_name);
	args.push_back(filename);

	return dosub("main::eval_file", &args);
}

int Embperl::dosub(const char* sub_name, const std::vector<std::string>* args, int mode)
{
	dSP;
	int         ret_value = 0;
	int         count;
	std::string error;

	ENTER;
	SAVETMPS;
	PUSHMARK(SP);

	if (args && !args->empty()) {
		for (auto i = args->begin(); i != args->end(); ++i) {
			XPUSHs(sv_2mortal(newSVpv(i->c_str(), i->length())));
		}
	}

	PUTBACK;

	count = call_pv(sub_name, mode);
	SPAGAIN;

	if (SvTRUE(ERRSV)) {
		error = SvPV_nolen(ERRSV);
		POPs;
	} else {
		if (count == 1) {
			SV* ret = POPs;
			if (SvTYPE(ret) == SVt_IV) {
				IV v = SvIV(ret);
				ret_value = v;
			}

			PUTBACK;
		}
	}

	FREETMPS;
	LEAVE;

	// not sure why we pass this as blind args, strange
	// check for syntax errors
	if (args && !args->empty()) {
		const std::string& filename = args->back();
		std::string sub = sub_name;
		if (sub == "main::eval_file" && !filename.empty() && File::Exists(filename)) {
			BenchTimer benchmark;

			std::string perl = "perl";
			if (File::Exists("/opt/eqemu-perl/bin/perl")) {
				perl = "/opt/eqemu-perl/bin/perl";
			}

			std::string syntax_error = Process::execute(
				fmt::format("{} -c {} 2>&1", perl, filename)
			);

			LogQuests("Perl eval [{}] took [{}]", filename, benchmark.elapsed());
			syntax_error = Strings::Trim(syntax_error);

			if (!Strings::Contains(syntax_error, "syntax OK")) {
				syntax_error += SvPVX(ERRSV);
				throw syntax_error;
			}
		}
	}

	if (error.length() > 0) {
		std::string errmsg = "Perl runtime error: ";
		errmsg += SvPVX(ERRSV);
		throw errmsg;
	}

	return ret_value;
}

//evaluate an expression. throw error on fail
int Embperl::eval(const char* code)
{
	std::vector<std::string> arg;
	arg.push_back(code);
	return dosub("main::my_eval", &arg, G_SCALAR | G_EVAL | G_KEEPERR);
}

bool Embperl::SubExists(const char* package, const char* sub)
{
	HV* stash = gv_stashpv(package, false);
	if (!stash) {
		return (false);
	}

	int len = strlen(sub);
	return (hv_exists(stash, sub, len));
}

#ifdef EMBPERL_IO_CAPTURE

XS(XS_EQEmuIO_PRINT); /* prototype to pass -Wmissing-prototypes */
XS(XS_EQEmuIO_PRINT)
{
	dXSARGS;
	if (items < 2) {
		return;
	}

	for (int r = 1; r < items; r++) {
		char* str = SvPV_nolen(ST(r));
		char* cur = str;

		/* Strip newlines from log message 'str' */
		*std::remove(str, str + strlen(str), '\n') = '\0';

		std::string log_string = str;

		if (
			log_string.find("did not return a true") != std::string::npos ||
			log_string.find("is experimental") != std::string::npos
		) {
			return;
		}

		for (int i = 0; *cur != '\0'; i++, cur++) {
			if (*cur == '\n') {
				LogQuests("{}", str);
			}
		}

		if (!log_string.empty()) {
			LogQuests("{}", log_string);
		}
	}

	XSRETURN_EMPTY;
}

#endif //EMBPERL_IO_CAPTURE

#endif //EMBPERL

#endif //EMBPERL_CPP
