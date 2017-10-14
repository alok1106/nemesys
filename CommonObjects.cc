#include "CommonObjects.hh"

#include <stdint.h>
#include <stdlib.h>

#include <unordered_map>
#include <vector>

#include "BuiltinFunctions.hh"
#include "Exception.hh"
#include "Types/Reference.hh"
#include "Types/Strings.hh"
#include "Types/List.hh"
#include "Types/Dictionary.hh"

using namespace std;


static const vector<const void*> objects({
  &MemoryError_instance,

  void_fn_ptr(&malloc),
  void_fn_ptr(&free),

  void_fn_ptr(&add_reference),
  void_fn_ptr(&delete_reference),

  void_fn_ptr(&_unwind_exception_internal),

  void_fn_ptr(&bytes_equal),
  void_fn_ptr(&bytes_compare),
  void_fn_ptr(&bytes_contains),
  void_fn_ptr(&bytes_concat),

  void_fn_ptr(&unicode_equal),
  void_fn_ptr(&unicode_compare),
  void_fn_ptr(&unicode_contains),
  void_fn_ptr(&unicode_concat),

  void_fn_ptr(&list_new),

  void_fn_ptr(&dictionary_next_item),
});

static unique_ptr<const unordered_map<const void*, size_t>> pointer_to_index;


const void* common_object_base() {
  return objects.data();
}

MemoryReference common_object_reference(const void* which) {
  if (!pointer_to_index.get()) {
    auto* m = new unordered_map<const void*, size_t>();
    for (size_t x = 0; x < objects.size(); x++) {
      m->emplace(objects[x], x);
    }
    pointer_to_index.reset(m);
  }

  return MemoryReference(Register::R12, pointer_to_index->at(which) * sizeof(void*));
}