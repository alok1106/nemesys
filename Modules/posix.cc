#include "posix.hh"

#include <inttypes.h>

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sysexits.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <memory>
#include <phosg/Filesystem.hh>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../Analysis.hh"
#include "../BuiltinFunctions.hh"
#include "../Types/Strings.hh"
#include "../Types/List.hh"
#include "../Types/Dictionary.hh"

using namespace std;



extern shared_ptr<GlobalAnalysis> global;

extern char** environ;

static wstring __doc__ = L"\
This module provides access to operating system functionality that is\n\
standardized by the C Standard and the POSIX standard (a thinly\n\
disguised Unix interface). Refer to the library manual and\n\
corresponding Unix manual entries for more information on calls.";

unordered_map<Variable, shared_ptr<Variable>> get_environ() {
  unordered_map<Variable, shared_ptr<Variable>> ret;
  for (char** env = environ; *env; env++) {
    size_t equals_loc;
    for (equals_loc = 0; (*env)[equals_loc] && ((*env)[equals_loc] != '='); equals_loc++);
    if (!(*env)[equals_loc]) {
      continue;
    }

    ret.emplace(piecewise_construct,
        forward_as_tuple(ValueType::Bytes, string(*env, equals_loc)),
        forward_as_tuple(new Variable(ValueType::Bytes, string(&(*env)[equals_loc + 1]))));
  }
  return ret;
}

unordered_map<Variable, shared_ptr<Variable>> sysconf_names({
  {Variable(ValueType::Unicode, L"SC_ARG_MAX"), shared_ptr<Variable>(
      new Variable(ValueType::Int, static_cast<int64_t>(_SC_ARG_MAX)))},
  {Variable(ValueType::Unicode, L"SC_CHILD_MAX"), shared_ptr<Variable>(
      new Variable(ValueType::Int, static_cast<int64_t>(_SC_CHILD_MAX)))},
  {Variable(ValueType::Unicode, L"SC_CLK_TCK"), shared_ptr<Variable>(
      new Variable(ValueType::Int, static_cast<int64_t>(_SC_CLK_TCK)))},
  {Variable(ValueType::Unicode, L"SC_IOV_MAX"), shared_ptr<Variable>(
      new Variable(ValueType::Int, static_cast<int64_t>(_SC_IOV_MAX)))},
  {Variable(ValueType::Unicode, L"SC_NGROUPS_MAX"), shared_ptr<Variable>(
      new Variable(ValueType::Int, static_cast<int64_t>(_SC_NGROUPS_MAX)))},
  {Variable(ValueType::Unicode, L"SC_NPROCESSORS_CONF"), shared_ptr<Variable>(
      new Variable(ValueType::Int, static_cast<int64_t>(_SC_NPROCESSORS_CONF)))},
  {Variable(ValueType::Unicode, L"SC_NPROCESSORS_ONLN"), shared_ptr<Variable>(
      new Variable(ValueType::Int, static_cast<int64_t>(_SC_NPROCESSORS_ONLN)))},
  {Variable(ValueType::Unicode, L"SC_OPEN_MAX"), shared_ptr<Variable>(
      new Variable(ValueType::Int, static_cast<int64_t>(_SC_OPEN_MAX)))},
  {Variable(ValueType::Unicode, L"SC_PAGESIZE"), shared_ptr<Variable>(
      new Variable(ValueType::Int, static_cast<int64_t>(_SC_PAGESIZE)))},
  {Variable(ValueType::Unicode, L"SC_STREAM_MAX"), shared_ptr<Variable>(
      new Variable(ValueType::Int, static_cast<int64_t>(_SC_STREAM_MAX)))},
  {Variable(ValueType::Unicode, L"SC_TZNAME_MAX"), shared_ptr<Variable>(
      new Variable(ValueType::Int, static_cast<int64_t>(_SC_TZNAME_MAX)))},
  {Variable(ValueType::Unicode, L"SC_JOB_CONTROL"), shared_ptr<Variable>(
      new Variable(ValueType::Int, static_cast<int64_t>(_SC_JOB_CONTROL)))},
  {Variable(ValueType::Unicode, L"SC_SAVED_IDS"), shared_ptr<Variable>(
      new Variable(ValueType::Int, static_cast<int64_t>(_SC_SAVED_IDS)))},
  {Variable(ValueType::Unicode, L"SC_VERSION"), shared_ptr<Variable>(
      new Variable(ValueType::Int, static_cast<int64_t>(_SC_VERSION)))},
  {Variable(ValueType::Unicode, L"SC_BC_BASE_MAX"), shared_ptr<Variable>(
      new Variable(ValueType::Int, static_cast<int64_t>(_SC_BC_BASE_MAX)))},
  {Variable(ValueType::Unicode, L"SC_BC_DIM_MAX"), shared_ptr<Variable>(
      new Variable(ValueType::Int, static_cast<int64_t>(_SC_BC_DIM_MAX)))},
  {Variable(ValueType::Unicode, L"SC_BC_SCALE_MAX"), shared_ptr<Variable>(
      new Variable(ValueType::Int, static_cast<int64_t>(_SC_BC_SCALE_MAX)))},
  {Variable(ValueType::Unicode, L"SC_BC_STRING_MAX"), shared_ptr<Variable>(
      new Variable(ValueType::Int, static_cast<int64_t>(_SC_BC_STRING_MAX)))},
  {Variable(ValueType::Unicode, L"SC_COLL_WEIGHTS_MAX"), shared_ptr<Variable>(
      new Variable(ValueType::Int, static_cast<int64_t>(_SC_COLL_WEIGHTS_MAX)))},
  {Variable(ValueType::Unicode, L"SC_EXPR_NEST_MAX"), shared_ptr<Variable>(
      new Variable(ValueType::Int, static_cast<int64_t>(_SC_EXPR_NEST_MAX)))},
  {Variable(ValueType::Unicode, L"SC_LINE_MAX"), shared_ptr<Variable>(
      new Variable(ValueType::Int, static_cast<int64_t>(_SC_LINE_MAX)))},
  {Variable(ValueType::Unicode, L"SC_RE_DUP_MAX"), shared_ptr<Variable>(
      new Variable(ValueType::Int, static_cast<int64_t>(_SC_RE_DUP_MAX)))},
  {Variable(ValueType::Unicode, L"SC_2_VERSION"), shared_ptr<Variable>(
      new Variable(ValueType::Int, static_cast<int64_t>(_SC_2_VERSION)))},
  {Variable(ValueType::Unicode, L"SC_2_C_BIND"), shared_ptr<Variable>(
      new Variable(ValueType::Int, static_cast<int64_t>(_SC_2_C_BIND)))},
  {Variable(ValueType::Unicode, L"SC_2_C_DEV"), shared_ptr<Variable>(
      new Variable(ValueType::Int, static_cast<int64_t>(_SC_2_C_DEV)))},
  {Variable(ValueType::Unicode, L"SC_2_CHAR_TERM"), shared_ptr<Variable>(
      new Variable(ValueType::Int, static_cast<int64_t>(_SC_2_CHAR_TERM)))},
  {Variable(ValueType::Unicode, L"SC_2_FORT_DEV"), shared_ptr<Variable>(
      new Variable(ValueType::Int, static_cast<int64_t>(_SC_2_FORT_DEV)))},
  {Variable(ValueType::Unicode, L"SC_2_FORT_RUN"), shared_ptr<Variable>(
      new Variable(ValueType::Int, static_cast<int64_t>(_SC_2_FORT_RUN)))},
  {Variable(ValueType::Unicode, L"SC_2_LOCALEDEF"), shared_ptr<Variable>(
      new Variable(ValueType::Int, static_cast<int64_t>(_SC_2_LOCALEDEF)))},
  {Variable(ValueType::Unicode, L"SC_2_SW_DEV"), shared_ptr<Variable>(
      new Variable(ValueType::Int, static_cast<int64_t>(_SC_2_SW_DEV)))},
  {Variable(ValueType::Unicode, L"SC_2_UPE"), shared_ptr<Variable>(
      new Variable(ValueType::Int, static_cast<int64_t>(_SC_2_UPE)))},
  {Variable(ValueType::Unicode, L"SC_PHYS_PAGES"), shared_ptr<Variable>(
      new Variable(ValueType::Int, static_cast<int64_t>(_SC_PHYS_PAGES)))},
});

static map<string, Variable> globals({
  {"__doc__",                Variable(ValueType::Unicode, __doc__)},
  {"__package__",            Variable(ValueType::Unicode, L"")},

  {"CLD_CONTINUED",          Variable(ValueType::Int, static_cast<int64_t>(CLD_CONTINUED))},
  {"CLD_DUMPED",             Variable(ValueType::Int, static_cast<int64_t>(CLD_DUMPED))},
  {"CLD_EXITED",             Variable(ValueType::Int, static_cast<int64_t>(CLD_EXITED))},
  {"CLD_TRAPPED",            Variable(ValueType::Int, static_cast<int64_t>(CLD_TRAPPED))},

  {"EX_CANTCREAT",           Variable(ValueType::Int, static_cast<int64_t>(EX_CANTCREAT))},
  {"EX_CONFIG",              Variable(ValueType::Int, static_cast<int64_t>(EX_CONFIG))},
  {"EX_DATAERR",             Variable(ValueType::Int, static_cast<int64_t>(EX_DATAERR))},
  {"EX_IOERR",               Variable(ValueType::Int, static_cast<int64_t>(EX_IOERR))},
  {"EX_NOHOST",              Variable(ValueType::Int, static_cast<int64_t>(EX_NOHOST))},
  {"EX_NOINPUT",             Variable(ValueType::Int, static_cast<int64_t>(EX_NOINPUT))},
  {"EX_NOPERM",              Variable(ValueType::Int, static_cast<int64_t>(EX_NOPERM))},
  {"EX_NOUSER",              Variable(ValueType::Int, static_cast<int64_t>(EX_NOUSER))},
  {"EX_OK",                  Variable(ValueType::Int, static_cast<int64_t>(EX_OK))},
  {"EX_OSERR",               Variable(ValueType::Int, static_cast<int64_t>(EX_OSERR))},
  {"EX_OSFILE",              Variable(ValueType::Int, static_cast<int64_t>(EX_OSFILE))},
  {"EX_PROTOCOL",            Variable(ValueType::Int, static_cast<int64_t>(EX_PROTOCOL))},
  {"EX_SOFTWARE",            Variable(ValueType::Int, static_cast<int64_t>(EX_SOFTWARE))},
  {"EX_TEMPFAIL",            Variable(ValueType::Int, static_cast<int64_t>(EX_TEMPFAIL))},
  {"EX_UNAVAILABLE",         Variable(ValueType::Int, static_cast<int64_t>(EX_UNAVAILABLE))},
  {"EX_USAGE",               Variable(ValueType::Int, static_cast<int64_t>(EX_USAGE))},

  {"F_LOCK",                 Variable(ValueType::Int, static_cast<int64_t>(F_LOCK))},
  {"F_OK",                   Variable(ValueType::Int, static_cast<int64_t>(F_OK))},
  {"F_TEST",                 Variable(ValueType::Int, static_cast<int64_t>(F_TEST))},
  {"F_TLOCK",                Variable(ValueType::Int, static_cast<int64_t>(F_TLOCK))},
  {"F_ULOCK",                Variable(ValueType::Int, static_cast<int64_t>(F_ULOCK))},

  {"O_ACCMODE",              Variable(ValueType::Int, static_cast<int64_t>(O_ACCMODE))},
  {"O_APPEND",               Variable(ValueType::Int, static_cast<int64_t>(O_APPEND))},
  {"O_ASYNC",                Variable(ValueType::Int, static_cast<int64_t>(O_ASYNC))},
  {"O_CLOEXEC",              Variable(ValueType::Int, static_cast<int64_t>(O_CLOEXEC))},
  {"O_CREAT",                Variable(ValueType::Int, static_cast<int64_t>(O_CREAT))},
  {"O_DIRECTORY",            Variable(ValueType::Int, static_cast<int64_t>(O_DIRECTORY))},
  {"O_DSYNC",                Variable(ValueType::Int, static_cast<int64_t>(O_DSYNC))},
  {"O_EXCL",                 Variable(ValueType::Int, static_cast<int64_t>(O_EXCL))},
  {"O_NDELAY",               Variable(ValueType::Int, static_cast<int64_t>(O_NDELAY))},
  {"O_NOCTTY",               Variable(ValueType::Int, static_cast<int64_t>(O_NOCTTY))},
  {"O_NOFOLLOW",             Variable(ValueType::Int, static_cast<int64_t>(O_NOFOLLOW))},
  {"O_NONBLOCK",             Variable(ValueType::Int, static_cast<int64_t>(O_NONBLOCK))},
  {"O_RDONLY",               Variable(ValueType::Int, static_cast<int64_t>(O_RDONLY))},
  {"O_RDWR",                 Variable(ValueType::Int, static_cast<int64_t>(O_RDWR))},
  {"O_SYNC",                 Variable(ValueType::Int, static_cast<int64_t>(O_SYNC))},
  {"O_TRUNC",                Variable(ValueType::Int, static_cast<int64_t>(O_TRUNC))},
  {"O_WRONLY",               Variable(ValueType::Int, static_cast<int64_t>(O_WRONLY))},
#ifdef MACOSX
  {"O_EXLOCK",               Variable(ValueType::Int, static_cast<int64_t>(O_EXLOCK))}, // osx only
  {"O_SHLOCK",               Variable(ValueType::Int, static_cast<int64_t>(O_SHLOCK))}, // osx only
#endif
#ifdef LINUX
  {"O_DIRECT",               Variable(ValueType::Int, static_cast<int64_t>(O_DIRECT))}, // linux only
  {"O_LARGEFILE",            Variable(ValueType::Int, static_cast<int64_t>(O_LARGEFILE))}, // linux only
  {"O_NOATIME",              Variable(ValueType::Int, static_cast<int64_t>(O_NOATIME))}, // linux only
  {"O_PATH",                 Variable(ValueType::Int, static_cast<int64_t>(O_PATH))}, // linux only
  {"O_RSYNC",                Variable(ValueType::Int, static_cast<int64_t>(O_RSYNC))}, // linux only
  {"O_TMPFILE",              Variable(ValueType::Int, static_cast<int64_t>(O_TMPFILE))}, // linux only
#endif

  {"environ",                Variable(ValueType::Dict, get_environ())},
  {"sysconf_names",          Variable(ValueType::Dict, sysconf_names)},

  // unimplemented stuff:

  // {"DirEntry", Variable()},
  // {"NGROUPS_MAX", Variable()},
  // {"POSIX_FADV_DONTNEED", Variable()}, // linux only
  // {"POSIX_FADV_NOREUSE", Variable()}, // linux only
  // {"POSIX_FADV_NORMAL", Variable()}, // linux only
  // {"POSIX_FADV_RANDOM", Variable()}, // linux only
  // {"POSIX_FADV_SEQUENTIAL", Variable()}, // linux only
  // {"POSIX_FADV_WILLNEED", Variable()}, // linux only
  // {"PRIO_PGRP", Variable()},
  // {"PRIO_PROCESS", Variable()},
  // {"PRIO_USER", Variable()},
  // {"P_ALL", Variable()},
  // {"P_PGID", Variable()},
  // {"P_PID", Variable()},
  // {"RTLD_DEEPBIND", Variable()}, // linux only
  // {"RTLD_GLOBAL", Variable()},
  // {"RTLD_LAZY", Variable()},
  // {"RTLD_LOCAL", Variable()},
  // {"RTLD_NODELETE", Variable()},
  // {"RTLD_NOLOAD", Variable()},
  // {"RTLD_NOW", Variable()},
  // {"R_OK", Variable()},
  // {"SCHED_BATCH", Variable()}, // linux only
  // {"SCHED_FIFO", Variable()},
  // {"SCHED_IDLE", Variable()}, // linux only
  // {"SCHED_OTHER", Variable()},
  // {"SCHED_RESET_ON_FORK", Variable()}, // linux only
  // {"SCHED_RR", Variable()},
  // {"SEEK_DATA", Variable()}, // linux only
  // {"SEEK_HOLE", Variable()}, // linux only
  // {"ST_APPEND", Variable()}, // linux only
  // {"ST_MANDLOCK", Variable()}, // linux only
  // {"ST_NOATIME", Variable()}, // linux only
  // {"ST_NODEV", Variable()}, // linux only
  // {"ST_NODIRATIME", Variable()}, // linux only
  // {"ST_NOEXEC", Variable()}, // linux only
  // {"ST_NOSUID", Variable()},
  // {"ST_RDONLY", Variable()},
  // {"ST_RELATIME", Variable()}, // linux only
  // {"ST_SYNCHRONOUS", Variable()}, // linux only
  // {"ST_WRITE", Variable()}, // linux only
  // {"TMP_MAX", Variable()},
  // {"WCONTINUED", Variable()},
  // {"WCOREDUMP", Variable()},
  // {"WEXITED", Variable()},
  // {"WEXITSTATUS", Variable()},
  // {"WIFCONTINUED", Variable()},
  // {"WIFEXITED", Variable()},
  // {"WIFSIGNALED", Variable()},
  // {"WIFSTOPPED", Variable()},
  // {"WNOHANG", Variable()},
  // {"WNOWAIT", Variable()},
  // {"WSTOPPED", Variable()},
  // {"WSTOPSIG", Variable()},
  // {"WTERMSIG", Variable()},
  // {"WUNTRACED", Variable()},
  // {"W_OK", Variable()},
  // {"XATTR_CREATE", Variable()}, // linux only
  // {"XATTR_REPLACE", Variable()}, // linux only
  // {"XATTR_SIZE_MAX", Variable()}, // linux only
  // {"X_OK", Variable()},

  // {"_have_functions", Variable(ValueType::List, ['HAVE_FACCESSAT', 'HAVE_FCHDIR', 'HAVE_FCHMOD', 'HAVE_FCHMODAT', 'HAVE_FCHOWN', 'HAVE_FCHOWNAT', 'HAVE_FDOPENDIR', 'HAVE_FPATHCONF', 'HAVE_FSTATAT', 'HAVE_FSTATVFS', 'HAVE_FTRUNCATE', 'HAVE_FUTIMES', 'HAVE_LINKAT', 'HAVE_LCHFLAGS', 'HAVE_LCHMOD', 'HAVE_LCHOWN', 'HAVE_LSTAT', 'HAVE_LUTIMES', 'HAVE_MKDIRAT', 'HAVE_OPENAT', 'HAVE_READLINKAT', 'HAVE_RENAMEAT', 'HAVE_SYMLINKAT', 'HAVE_UNLINKAT'])},
});

std::shared_ptr<ModuleAnalysis> posix_module(new ModuleAnalysis("posix", globals));

static void raise_OSError(ExceptionBlock* exc_block, int64_t error_code) {
  raise_python_exception(exc_block, create_single_attr_instance(
      OSError_class_id, static_cast<int64_t>(error_code)));
}

#define simple_wrapper(call, ...) void_fn_ptr([](__VA_ARGS__) -> int64_t { \
    int64_t ret = call; \
    if (ret < 0) { \
      raise_OSError(exc_block, errno); \
    } \
    return ret; \
  })

void posix_initialize() {
  Variable Bool(ValueType::Bool);
  Variable Bool_True(ValueType::Bool, true);
  Variable Bool_False(ValueType::Bool, false);
  Variable Int(ValueType::Int);
  Variable Float(ValueType::Float);
  Variable Bytes(ValueType::Bytes);
  Variable Unicode(ValueType::Unicode);
  Variable List_Unicode(ValueType::List, vector<Variable>({Unicode}));
  Variable Dict_Unicode_Unicode(ValueType::Dict, vector<Variable>({Unicode, Unicode}));
  Variable None(ValueType::None);

  BuiltinClassDefinition stat_result_def("stat_result", {
        {"st_mode", Int},
        {"st_ino", Int},
        {"st_dev", Int},
        {"st_nlink", Int},
        {"st_uid", Int},
        {"st_gid", Int},
        {"st_size", Int},
        {"st_atime", Float},
        {"st_mtime", Float},
        {"st_ctime", Float},
        {"st_atime_ns", Int},
        {"st_mtime_ns", Int},
        {"st_ctime_ns", Int},
        {"st_blocks", Int},
        {"st_blksize", Int},
        {"st_rdev", Int}},
      {}, void_fn_ptr(&free), false);

  // note: we don't create stat_result within posix_module because it doesn't
  // have an __init__ function, so it isn't constructible from python code
  static int64_t stat_result_class_id = create_builtin_class(stat_result_def);
  Variable StatResult(ValueType::Instance, stat_result_class_id, NULL);
  static ClassContext* stat_result_class = global->context_for_class(stat_result_class_id);

  static auto convert_stat_result = +[](const struct stat* st) -> void* {
    // TODO: this can be optimized to avoid all the map lookups
    InstanceObject* res = create_instance(stat_result_class_id, stat_result_class->attribute_count());
    stat_result_class->set_attribute(res, "st_mode", st->st_mode);
    stat_result_class->set_attribute(res, "st_ino", st->st_ino);
    stat_result_class->set_attribute(res, "st_dev", st->st_dev);
    stat_result_class->set_attribute(res, "st_nlink", st->st_nlink);
    stat_result_class->set_attribute(res, "st_uid", st->st_uid);
    stat_result_class->set_attribute(res, "st_gid", st->st_gid);
    stat_result_class->set_attribute(res, "st_size", st->st_size);
    stat_result_class->set_attribute(res, "st_blocks", st->st_blocks);
    stat_result_class->set_attribute(res, "st_blksize", st->st_blksize);
    stat_result_class->set_attribute(res, "st_rdev", st->st_rdev);

#ifdef MACOSX
    stat_result_class->set_attribute(res, "st_atime_ns", st->st_atimespec.tv_sec * 1000000000 + st->st_atimespec.tv_nsec);
    stat_result_class->set_attribute(res, "st_mtime_ns", st->st_mtimespec.tv_sec * 1000000000 + st->st_mtimespec.tv_nsec);
    stat_result_class->set_attribute(res, "st_ctime_ns", st->st_ctimespec.tv_sec * 1000000000 + st->st_ctimespec.tv_nsec);
    double atime = static_cast<double>(st->st_atimespec.tv_sec * 1000000000 + st->st_atimespec.tv_nsec) / 1000000000;
    double mtime = static_cast<double>(st->st_mtimespec.tv_sec * 1000000000 + st->st_mtimespec.tv_nsec) / 1000000000;
    double ctime = static_cast<double>(st->st_ctimespec.tv_sec * 1000000000 + st->st_ctimespec.tv_nsec) / 1000000000;
#else // LINUX
    stat_result_class->set_attribute(res, "st_atime_ns", st->st_atim.tv_sec * 1000000000 + st->st_atim.tv_nsec);
    stat_result_class->set_attribute(res, "st_mtime_ns", st->st_mtim.tv_sec * 1000000000 + st->st_mtim.tv_nsec);
    stat_result_class->set_attribute(res, "st_ctime_ns", st->st_ctim.tv_sec * 1000000000 + st->st_ctim.tv_nsec);
    double atime = static_cast<double>(st->st_atim.tv_sec * 1000000000 + st->st_atim.tv_nsec) / 1000000000;
    double mtime = static_cast<double>(st->st_mtim.tv_sec * 1000000000 + st->st_mtim.tv_nsec) / 1000000000;
    double ctime = static_cast<double>(st->st_ctim.tv_sec * 1000000000 + st->st_ctim.tv_nsec) / 1000000000;
#endif

    stat_result_class->set_attribute(res, "st_atime", *reinterpret_cast<int64_t*>(&atime));
    stat_result_class->set_attribute(res, "st_mtime", *reinterpret_cast<int64_t*>(&mtime));
    stat_result_class->set_attribute(res, "st_ctime", *reinterpret_cast<int64_t*>(&ctime));

    return res;
  };

  vector<BuiltinFunctionDefinition> module_function_defs({
    {"getpid", {}, Int, void_fn_ptr(&getpid), false, false},
    {"getppid", {}, Int, void_fn_ptr(&getppid), false, false},
    {"getpgid", {Int}, Int, void_fn_ptr(&getpgid), false, false},
    {"getpgrp", {}, Int, void_fn_ptr(&getpgrp), false, false},
    {"getsid", {Int}, Int, void_fn_ptr(&getsid), false, false},

    {"getuid", {}, Int, void_fn_ptr(&getuid), false, false},
    {"getgid", {}, Int, void_fn_ptr(&getgid), false, false},
    {"geteuid", {}, Int, void_fn_ptr(&geteuid), false, false},
    {"getegid", {}, Int, void_fn_ptr(&getegid), false, false},

    // these functions never return, so return type is technically unused
    {"_exit", {Int}, Int, void_fn_ptr(&_exit), false, false},
    {"abort", {}, Int, void_fn_ptr(&abort), false, false},

    {"close", {Int}, None, simple_wrapper(close(fd), int64_t fd, ExceptionBlock* exc_block), true, false},

    {"closerange", {Int, Int}, None, void_fn_ptr([](int64_t fd, int64_t end_fd) {
      for (; fd < end_fd; fd++) {
        close(fd);
      }
    }), false, false},

    {"dup", {Int}, Int, simple_wrapper(dup(fd), int64_t fd, ExceptionBlock* exc_block), true, false},
    {"dup2", {Int}, Int, simple_wrapper(dup2(fd, new_fd), int64_t fd, int64_t new_fd, ExceptionBlock* exc_block), true, false},

    {"fork", {}, Int, simple_wrapper(fork(), ExceptionBlock* exc_block), true, false},

    {"kill", {Int, Int}, Int, simple_wrapper(kill(pid, sig), int64_t pid, int64_t sig, ExceptionBlock* exc_block), true, false},
    {"killpg", {Int, Int}, Int, simple_wrapper(killpg(pid, sig), int64_t pid, int64_t sig, ExceptionBlock* exc_block), true, false},

    {"open", {Unicode, Int, Variable(ValueType::Int, static_cast<int64_t>(0777))}, Int,
        void_fn_ptr([](UnicodeObject* path, int64_t flags, int64_t mode, ExceptionBlock* exc_block) -> int64_t {
      BytesObject* path_bytes = unicode_encode_ascii(path);
      delete_reference(path);

      int64_t ret = open(path_bytes->data, flags, mode);
      delete_reference(path_bytes);

      if (ret < 0) {
        raise_OSError(exc_block, errno);
      }

      return ret;
    }), true, false},

    {"read", {Int, Int}, Bytes, void_fn_ptr([](int64_t fd, int64_t buffer_size, ExceptionBlock* exc_block) -> BytesObject* {
      BytesObject* ret = bytes_new(NULL, buffer_size);
      ssize_t bytes_read = read(fd, ret->data, buffer_size);
      if (bytes_read >= 0) {
        ret->count = bytes_read;
      } else {
        delete_reference(ret);
        raise_OSError(exc_block, errno);
      }
      return ret;
    }), true, false},

    {"write", {Int, Bytes}, Int, void_fn_ptr([](int64_t fd, BytesObject* data, ExceptionBlock* exc_block) -> int64_t {
      ssize_t bytes_written = write(fd, data->data, data->count);
      delete_reference(data);
      if (bytes_written < 0) {
        raise_OSError(exc_block, errno);
      }
      return bytes_written;
    }), true, false},

    {"execv", {Unicode, List_Unicode}, None, void_fn_ptr([](UnicodeObject* path, ListObject* args, ExceptionBlock* exc_block) {

      BytesObject* path_bytes = unicode_encode_ascii(path);

      vector<BytesObject*> args_objects;
      vector<char*> args_pointers;
      for (size_t x = 0; x < args->count; x++) {
        args_objects.emplace_back(unicode_encode_ascii(reinterpret_cast<UnicodeObject*>(args->items[x])));
        args_pointers.emplace_back(args_objects.back()->data);
      }
      args_pointers.emplace_back(nullptr);

      execv(path_bytes->data, args_pointers.data());

      // little optimization: we expect execv to succeed most of the time, so we
      // don't bother deleting path until after it returns (and has failed)
      delete_reference(path);
      delete_reference(path_bytes);
      for (auto& o : args_objects) {
        delete_reference(o);
      }

      raise_OSError(exc_block, errno);
    }), true, false},

    {"execve", {Unicode, List_Unicode, Dict_Unicode_Unicode}, None,
        void_fn_ptr([](UnicodeObject* path, ListObject* args,
          DictionaryObject* env, ExceptionBlock* exc_block) {

      BytesObject* path_bytes = unicode_encode_ascii(path);

      vector<BytesObject*> args_objects;
      vector<char*> args_pointers;
      for (size_t x = 0; x < args->count; x++) {
        args_objects.emplace_back(unicode_encode_ascii(reinterpret_cast<UnicodeObject*>(args->items[x])));
        args_pointers.emplace_back(args_objects.back()->data);
      }
      args_pointers.emplace_back(nullptr);

      vector<char*> envs_pointers;
      DictionaryObject::SlotContents dsc;
      while (dictionary_next_item(env, &dsc)) {
        BytesObject* key_bytes = unicode_encode_ascii(reinterpret_cast<UnicodeObject*>(dsc.key));
        BytesObject* value_bytes = unicode_encode_ascii(reinterpret_cast<UnicodeObject*>(dsc.value));

        char* env_item = reinterpret_cast<char*>(malloc(key_bytes->count + value_bytes->count + 2));
        memcpy(env_item, key_bytes->data, key_bytes->count);
        env_item[key_bytes->count] = '=';
        memcpy(&env_item[key_bytes->count + 1], value_bytes->data, value_bytes->count);
        env_item[key_bytes->count + value_bytes->count + 1] = 0;
        envs_pointers.emplace_back(env_item);

        delete_reference(key_bytes);
        delete_reference(value_bytes);
      }
      envs_pointers.emplace_back(nullptr);

      execve(path_bytes->data, args_pointers.data(), envs_pointers.data());

      // little optimization: we expect execve to succeed most of the time, so we
      // don't bother deleting path until after it returns (and has failed)
      delete_reference(path);
      delete_reference(path_bytes);
      for (auto& o : args_objects) {
        delete_reference(o);
      }
      envs_pointers.pop_back(); // the last one is NULL
      for (auto& ptr : envs_pointers) {
        free(ptr);
      }

      raise_OSError(exc_block, errno);
    }), true, false},

    {"strerror", {Int}, Unicode, void_fn_ptr([](int64_t code) -> UnicodeObject* {
      char buf[128];

#ifdef LINUX
      // linux has a "feature" where there are two different versions of
      // strerror_r. the GNU-specific one returns a char*, which might not point
      // to buf, probbaly to avoid copying. we can't use the XSI-compliant one
      // because apparantely libstdc++ requires _GNU_SOURCE to be defined, which
      // enables the GNU-specific version of strerror_r. so this means we have
      // to expect buf to be unused sometimes, sigh
      return bytes_decode_ascii(strerror_r(code, buf, sizeof(buf)));
#else
      strerror_r(code, buf, sizeof(buf));
      return bytes_decode_ascii(buf);
#endif
    }), false, false},

    // TODO: most functions below here should raise OSError on failure instead
    // of returning errno
    {"access", {Unicode, Int}, Int,
        void_fn_ptr([](UnicodeObject* path, int64_t mode) -> int64_t {
      BytesObject* path_bytes = unicode_encode_ascii(path);
      delete_reference(path);

      int64_t ret = access(path_bytes->data, mode);
      delete_reference(path_bytes);
      return ret;
    }), false, false},

    {"chdir", {Unicode}, Int, void_fn_ptr([](UnicodeObject* path) -> int64_t {
      BytesObject* path_bytes = unicode_encode_ascii(path);
      delete_reference(path);

      int64_t ret = chdir(path_bytes->data);
      delete_reference(path_bytes);
      return ret;
    }), false, false},

    {"fchdir", {Int}, Int, void_fn_ptr(&fchdir), false, false},

    {"chmod", {Unicode, Int}, Int, void_fn_ptr([](UnicodeObject* path, int64_t mode) -> int64_t {
      BytesObject* path_bytes = unicode_encode_ascii(path);
      delete_reference(path);

      int64_t ret = chmod(path_bytes->data, mode);
      delete_reference(path_bytes);
      return ret;
    }), false, false},

    {"fchmod", {Int, Int}, Int, void_fn_ptr(&fchmod), false, false},

#ifdef MACOSX
    {"chflags", {Unicode, Int}, Int,
        void_fn_ptr([](UnicodeObject* path, int64_t flags) -> int64_t {
      BytesObject* path_bytes = unicode_encode_ascii(path);
      delete_reference(path);

      int64_t ret = chflags(path_bytes->data, flags);
      delete_reference(path_bytes);
      return ret;
    }), false, false},

    {"fchflags", {Int, Int}, Int, void_fn_ptr(&fchflags), false, false},
#endif

    {"chown", {Unicode, Int, Int}, Int, void_fn_ptr([](UnicodeObject* path, int64_t uid, int64_t gid) -> int64_t {
      BytesObject* path_bytes = unicode_encode_ascii(path);
      delete_reference(path);

      int64_t ret = chown(path_bytes->data, uid, gid);
      delete_reference(path_bytes);
      return ret;
    }), false, false},

    {"lchown", {Unicode, Int, Int}, Int,
        void_fn_ptr([](UnicodeObject* path, int64_t uid, int64_t gid) -> int64_t {
      BytesObject* path_bytes = unicode_encode_ascii(path);
      delete_reference(path);

      int64_t ret = lchown(path_bytes->data, uid, gid);
      delete_reference(path_bytes);
      return ret;
    }), false, false},

    {"fchown", {Int, Int, Int}, Int, void_fn_ptr(&fchown), false, false},

    {"chroot", {Unicode}, Int, void_fn_ptr([](UnicodeObject* path) -> int64_t {
      BytesObject* path_bytes = unicode_encode_ascii(path);
      delete_reference(path);

      int64_t ret = chroot(path_bytes->data);
      delete_reference(path_bytes);
      return ret;
    }), false, false},

    {"ctermid", {}, Unicode, void_fn_ptr([]() -> UnicodeObject* {
      char result[L_ctermid];
      ctermid(result);
      return bytes_decode_ascii(result);
    }), false, false},

    {"cpu_count", {}, Int, void_fn_ptr([]() -> int64_t {
      // TODO: should we use procfs here instead?
      return sysconf(_SC_NPROCESSORS_ONLN);
    }), false, false},

    // TODO: support dir_fd
    {"stat", {Unicode, Bool_True}, StatResult,
        void_fn_ptr([](UnicodeObject* path, bool follow_symlinks, ExceptionBlock* exc_block) -> void* {
      BytesObject* path_bytes = unicode_encode_ascii(path);
      delete_reference(path);

      struct stat st;
      int64_t ret = follow_symlinks ? stat(path_bytes->data, &st) : lstat(path_bytes->data, &st);
      delete_reference(path_bytes);

      if (ret) {
        raise_OSError(exc_block, errno);
      }
      return convert_stat_result(&st);
    }), true, false},

    {"fstat", {Int}, StatResult, void_fn_ptr([](int64_t fd, ExceptionBlock* exc_block) -> void* {
      struct stat st;
      if (fstat(fd, &st)) {
        raise_OSError(exc_block, errno);
      }
      return convert_stat_result(&st);
    }), true, false},

    {"truncate", {Unicode, Int}, Int, void_fn_ptr([](UnicodeObject* path, int64_t size) -> int64_t {
      BytesObject* path_bytes = unicode_encode_ascii(path);
      delete_reference(path);

      int64_t ret = truncate(path_bytes->data, size);
      delete_reference(path_bytes);
      return ret;
    }), false, false},

    {"ftruncate", {Int, Int}, Int, void_fn_ptr(&ftruncate), false, false},

    {"getcwd", {}, Unicode,
        void_fn_ptr([](ExceptionBlock* exc_block) -> UnicodeObject* {
      char path[MAXPATHLEN];
      if (!getcwd(path, MAXPATHLEN)) {
        raise_OSError(exc_block, errno);
      }
      return bytes_decode_ascii(path);
    }), true, false},

    {"getcwdb", {}, Bytes, void_fn_ptr([](ExceptionBlock* exc_block) -> BytesObject* {
      BytesObject* ret = bytes_new(NULL, MAXPATHLEN);
      if (!getcwd(ret->data, MAXPATHLEN)) {
        delete_reference(ret);
        raise_OSError(exc_block, errno);
      } else {
        ret->count = strlen(ret->data);
      }
      return ret;
    }), true, false},

    {"lseek", {Int, Int, Int}, Int, void_fn_ptr(&lseek), false, false},
    {"fsync", {Int}, Int, void_fn_ptr(&fsync), false, false},
    {"isatty", {Int}, Bool, void_fn_ptr(&isatty), false, false},

    {"listdir", {Variable(ValueType::Unicode, L".")}, List_Unicode,
        void_fn_ptr([](UnicodeObject* path) -> void* {
      BytesObject* path_bytes = unicode_encode_ascii(path);
      delete_reference(path);

      // TODO: we shouldn't use list_directory here because it can throw c++
      // exceptions; that will break nemesys-generated code
      auto items = list_directory(path_bytes->data);
      delete_reference(path_bytes);

      ListObject* l = list_new(items.size(), true);
      size_t x = 0;
      for (const auto& item : items) {
        l->items[x++] = bytes_decode_ascii(item.c_str());
      }

      return l;
    }), false, false},

    // TODO: support passing names as strings
    {"sysconf", {Int}, Int, void_fn_ptr(&sysconf), false, false},

    // {"confstr", Variable()},
    // {"confstr_names", Variable()},
    // {"device_encoding", Variable()},
    // {"error", Variable()},
    // {"fdatasync", Variable()}, // linux only
    // {"forkpty", Variable()},
    // {"fpathconf", Variable()},
    // {"fspath", Variable()},
    // {"fstatvfs", Variable()},
    // {"get_blocking", Variable()},
    // {"get_inheritable", Variable()},
    // {"get_terminal_size", Variable()},
    // {"getgrouplist", Variable()},
    // {"getgroups", Variable()},
    // {"getloadavg", Variable()},
    // {"getlogin", Variable()},
    // {"getpriority", Variable()},
    // {"getresgid", Variable()}, // linux only
    // {"getresuid", Variable()}, // linux only
    // {"getxattr", Variable()}, // linux only
    // {"initgroups", Variable()},
    // {"lchflags", Variable()}, // osx only
    // {"lchmod", Variable()}, // osx only
    // {"link", Variable()},
    // {"listxattr", Variable()}, // linux only
    // {"lockf", Variable()},
    // {"major", Variable()},
    // {"makedev", Variable()},
    // {"minor", Variable()},
    // {"mkdir", Variable()},
    // {"mkfifo", Variable()},
    // {"mknod", Variable()},
    // {"nice", Variable()},
    // {"openpty", Variable()},
    // {"pathconf", Variable()},
    // {"pathconf_names", Variable()},
    // {"pipe", Variable()},
    // {"pipe2", Variable()}, // linux only
    // {"posix_fadvise", Variable()}, // linux only
    // {"posix_fallocate", Variable()}, // linux only
    // {"pread", Variable()},
    // {"putenv", Variable()},
    // {"pwrite", Variable()},
    // {"readlink", Variable()},
    // {"readv", Variable()},
    // {"remove", Variable()},
    // {"removexattr", Variable()}, // linux only
    // {"rename", Variable()},
    // {"replace", Variable()},
    // {"rmdir", Variable()},
    // {"scandir", Variable()},
    // {"sched_get_priority_max", Variable()},
    // {"sched_get_priority_min", Variable()},
    // {"sched_getaffinity", Variable()}, // linux only
    // {"sched_getparam", Variable()}, // linux only
    // {"sched_getscheduler", Variable()}, // linux only
    // {"sched_param", Variable()}, // linux only
    // {"sched_rr_get_interval", Variable()}, // linux only
    // {"sched_setaffinity", Variable()}, // linux only
    // {"sched_setparam", Variable()}, // linux only
    // {"sched_setscheduler", Variable()}, // linux only
    // {"sched_yield", Variable()},
    // {"sendfile", Variable()},
    // {"set_blocking", Variable()},
    // {"set_inheritable", Variable()},
    // {"setegid", Variable()},
    // {"seteuid", Variable()},
    // {"setgid", Variable()},
    // {"setgroups", Variable()},
    // {"setpgid", Variable()},
    // {"setpgrp", Variable()},
    // {"setpriority", Variable()},
    // {"setregid", Variable()},
    // {"setresgid", Variable()}, // linux only
    // {"setresuid", Variable()}, // linux only
    // {"setreuid", Variable()},
    // {"setsid", Variable()},
    // {"setuid", Variable()},
    // {"setxattr", Variable()}, // linux only
    // {"stat_float_times", Variable()},
    // {"statvfs", Variable()},
    // {"statvfs_result", Variable()},
    // {"symlink", Variable()},
    // {"sync", Variable()},
    // {"system", Variable()},
    // {"tcgetpgrp", Variable()},
    // {"tcsetpgrp", Variable()},
    // {"terminal_size", Variable()},
    // {"times", Variable()},
    // {"times_result", Variable()},
    // {"ttyname", Variable()},
    // {"umask", Variable()},
    // {"uname", Variable()},
    // {"uname_result", Variable()},
    // {"unlink", Variable()},
    // {"unsetenv", Variable()},
    // {"urandom", Variable()},
    // {"utime", Variable()},
    // {"wait", Variable()},
    // {"wait3", Variable()},
    // {"wait4", Variable()},
    // {"waitid", Variable()}, // linux only
    // {"waitid_result", Variable()}, // linux only
    // {"waitpid", Variable()},
    // {"writev", Variable()},
  });

  for (auto& def : module_function_defs) {
    posix_module->create_builtin_function(def);
  }
}
