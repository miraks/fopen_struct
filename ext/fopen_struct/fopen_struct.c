#include <ruby.h>

VALUE cFOpenStruct;
VALUE cFOpenStructSingleton;
VALUE cSet;

static ID id_puts;
static ID id_each_pair;
static ID id_to_enum;
static ID id_to_a;
static ID id_battery;
static ID id_hash;
static ID id_map;
static ID id_sort;
static ID id_default;
static ID id_ivar_for_name;
static ID id_ivar_for_setter;

static VALUE rb_puts(VALUE obj) {
  return rb_funcall(rb_mKernel, id_puts, 1, obj);
}

static VALUE map_to_sym_block(VALUE key_value, VALUE fos_class, int argc, VALUE* argv) {
  VALUE key;

  key = rb_ary_entry(key_value, 0);

  return ID2SYM(rb_to_id(key));
}

static VALUE fos_class_create_class(VALUE self, VALUE keys) {
  VALUE klass, singleton;
  long keys_count;
  int i;

  klass = rb_define_class_id(0, self);
  singleton = rb_singleton_class(klass);
  keys_count = RARRAY_LEN(keys);
  VALUE* keys_array = RARRAY_PTR(keys);

  for (i = 0; i < keys_count; i++) {
    rb_define_attr(klass, rb_id2name(SYM2ID(keys_array[i])), 1, 1);
  }

  rb_define_alias(singleton, "new", "old_new");

  return klass;
}

static VALUE fos_class_new(int argc, VALUE* argv, VALUE self) {
  VALUE table, keys, cache, cached_klass;

  if (argc == 0) {
    table = rb_hash_new();
  } else {
    table = argv[0];
  }

  keys = rb_funcall(rb_block_call(rb_funcall(table, id_each_pair, 0), id_map, 0, 0, map_to_sym_block, self), id_sort, 0);

  cache = rb_iv_get(self, "@cache");
  cached_klass = rb_hash_aref(cache, keys);

  if(NIL_P(cached_klass)) {
    cached_klass = fos_class_create_class(self, keys);
    rb_hash_aset(cache, keys, cached_klass);
  }

  return rb_class_new_instance(argc, argv, cached_klass);
}

static VALUE fos_apparent_class(VALUE self) {
  VALUE klass;

  klass = rb_class_of(self);

  while(NIL_P(rb_mod_name(klass))) {
    klass = rb_class_get_superclass(klass);
  }

  return klass;
}

static VALUE fos_ivar_for_name(VALUE self, VALUE name) {
  VALUE ivar_name, cv_ivar_for_names;

  cv_ivar_for_names = rb_cv_get(rb_class_of(self), "@@ivar_for_names");
  ivar_name = rb_hash_aref(cv_ivar_for_names, name);

  if (NIL_P(ivar_name)) {
    const char* str_name;
    if (SYMBOL_P(name)) {
      str_name = rb_id2name(SYM2ID(name));
    } else {
      str_name = StringValuePtr(name);
    }

    char buf[strlen(str_name) + 2];

    snprintf(buf, sizeof buf, "%s%s", "@", str_name);
    ivar_name = ID2SYM(rb_intern(buf));
    rb_hash_aset(cv_ivar_for_names, name, ivar_name);
  }

  return ivar_name;
}

static VALUE set_table_variables_block(VALUE key_value, VALUE fos, int argc, VALUE* argv) {
  VALUE key, value, name;

  key = rb_ary_entry(key_value, 0);
  value = rb_ary_entry(key_value, 1);
  name = fos_ivar_for_name(fos, key);
  rb_ivar_set(fos, SYM2ID(name), value);

  return Qnil;
}

static VALUE fos_ivar_for_setter(VALUE self, VALUE name) {
  VALUE ivar_name, cv_ivar_for_names;

  cv_ivar_for_names = rb_cv_get(rb_class_of(self), "@@ivar_for_names");
  ivar_name = rb_hash_aref(cv_ivar_for_names, name);

  if (!NIL_P(ivar_name)) {
    return ivar_name;
  }

  const char* str_name;
  if (SYMBOL_P(name)) {
    str_name = rb_id2name(SYM2ID(name));
  } else {
    str_name = StringValuePtr(name);
  }

  size_t str_name_length = strlen(str_name);
  char buf[str_name_length + 1];
  char name_without_eq[str_name_length];

  strncpy(name_without_eq, str_name, str_name_length - 1);
  snprintf(buf, sizeof buf, "%s%s", "@", name_without_eq);

  ivar_name = ID2SYM(rb_intern(buf));
  rb_hash_aset(cv_ivar_for_names, name, ivar_name);

  return ivar_name;
}

static VALUE fos_get(VALUE self, VALUE name) {
  ID ivar_id;
  VALUE ivar;

  ivar = fos_ivar_for_name(self, name);
  ivar_id = SYM2ID(ivar);

  if (rb_ivar_defined(self, ivar_id)) {
    return rb_ivar_get(self, ivar_id);
  }

  return Qnil;
}

static VALUE fos_set(VALUE self, VALUE name, VALUE value) {
  ID ivar_id;
  VALUE ivar;

  ivar = fos_ivar_for_name(self, name);
  ivar_id = SYM2ID(ivar);

  rb_ivar_set(self, ivar_id, value);

  return value;
}

static VALUE fos_delete_field(VALUE self, VALUE name) {
  ID ivar_id;
  VALUE ivar, value;

  ivar = fos_ivar_for_name(self, name);
  ivar_id = SYM2ID(ivar);

  value = rb_ivar_get(self, ivar_id);
  rb_obj_remove_instance_variable(self, ivar);

  return value;
}

static VALUE fos_each_pair(VALUE self) {
  if (!rb_block_given_p()) {
    return rb_funcall(self, id_to_enum, 1, ID2SYM(id_each_pair));
  }

  VALUE iv, iv_name, ivs;
  ID iv_id;
  VALUE key_value;
  const char* str_name_orig;
  size_t str_name_length;
  int i;
  ivs = rb_obj_instance_variables(self);
  long ivs_count = RARRAY_LEN(ivs);

  for (i = 0; i < ivs_count; i++) {
    iv_name = rb_ary_entry(ivs, i);
    iv_id = SYM2ID(iv_name);

    if (iv_id == rb_intern("@method_missing_calls_count")) {
      continue;
    }

    iv = rb_ivar_get(self, iv_id);

    str_name_orig = rb_id2name(iv_id);
    str_name_length = strlen(str_name_orig);
    char str_name[str_name_length];
    strcpy(str_name, str_name_orig + 1);

    key_value = rb_ary_new2(2);
    rb_ary_push(key_value, ID2SYM(rb_intern(str_name)));
    rb_ary_push(key_value, iv);

    rb_yield(key_value);
  }

  return self;
}

static VALUE fos_equal(VALUE self, VALUE other) {
  if (fos_apparent_class(other) != cFOpenStruct) {
    return Qfalse;
  }

  VALUE self_ivs, other_ivs;
  long self_ivs_count, other_ivs_count;

  self_ivs = rb_obj_instance_variables(self);
  other_ivs = rb_obj_instance_variables(other);
  self_ivs_count = RARRAY_LEN(self_ivs);
  other_ivs_count = RARRAY_LEN(other_ivs);

  if (self_ivs_count != other_ivs_count) {
    return Qfalse;
  }

  VALUE iv_name, self_iv, other_iv;
  ID iv_id;
  int i;

  for (i = 0; i < self_ivs_count; i++) {
    iv_name = rb_ary_entry(self_ivs, i);
    iv_id = SYM2ID(iv_name);
    self_iv = rb_ivar_get(self, iv_id);
    other_iv = rb_ivar_get(other, iv_id);
    if (!rb_equal(self_iv, other_iv)) {
      return Qfalse;
    }
  }

  return Qtrue;
}

static VALUE fos_eql(VALUE self, VALUE other) {
  if (fos_apparent_class(other) != cFOpenStruct) {
    return Qfalse;
  }

  VALUE self_ivs, other_ivs;
  long self_ivs_count, other_ivs_count;

  self_ivs = rb_obj_instance_variables(self);
  other_ivs = rb_obj_instance_variables(other);
  self_ivs_count = RARRAY_LEN(self_ivs);
  other_ivs_count = RARRAY_LEN(other_ivs);

  if (self_ivs_count != other_ivs_count) {
    return Qfalse;
  }

  VALUE iv_name, self_iv, other_iv;
  ID iv_id;
  int i;

  for (i = 0; i < self_ivs_count; i++) {
    iv_name = rb_ary_entry(self_ivs, i);
    iv_id = SYM2ID(iv_name);
    self_iv = rb_ivar_get(self, iv_id);
    other_iv = rb_ivar_get(other, iv_id);
    if (!rb_eql(self_iv, other_iv)) {
      return Qfalse;
    }
  }

  return Qtrue;
}

static VALUE fos_inspect(VALUE self) {
  VALUE str;

  // later

  return str;
}

static VALUE fos_to_a(VALUE self) {
  return rb_funcall(fos_each_pair(self), id_to_a, 0);
}

static VALUE fos_to_h(VALUE self) {
  // Rewrite it later
  return rb_funcall(rb_cHash, id_battery, 1, fos_to_a(self));
}

static VALUE fos_hash(VALUE self) {
  return rb_funcall(fos_to_h(self), id_hash, 0);
}

static VALUE fos_respond_to_p(VALUE self, VALUE name) {
  VALUE ivar;
  ID ivar_id;
  const char* str_name_orig;
  long str_name_length;

  str_name_orig = rb_id2name(SYM2ID(name));
  str_name_length = strlen(str_name_orig);
  char str_name[str_name_length];
  strcpy(str_name, str_name_orig);

  if (str_name[str_name_length - 1] == '=') {
    str_name[str_name_length - 1] = 0;
  }

  ivar = fos_ivar_for_name(self, ID2SYM(rb_intern(str_name)));
  ivar_id = SYM2ID(ivar);
  if (rb_ivar_defined(self, ivar_id)) {
    return Qtrue;
  }

  VALUE argv[1];
  argv[0] = name;

  return rb_call_super(1, argv);
}

static void define_variable_accessor(VALUE self, VALUE name) {
  VALUE klass;

  klass = rb_class_of(self);
  rb_define_attr(klass, rb_id2name(SYM2ID(name)), 1, 1);
}

static void increase_missing_calls_count(VALUE self, VALUE name) {
  VALUE missing_calls_count, count, calls_until_define_accessor;
  const char* str_name_orig;
  long str_name_length;

  str_name_orig = rb_id2name(SYM2ID(name));
  str_name_length = strlen(str_name_orig);
  char str_name[str_name_length];
  strncpy(str_name, str_name_orig + 1, str_name_length);
  str_name_length = strlen(str_name);
  if (str_name[str_name_length - 1] == '=') {
    str_name[str_name_length - 1] = 0;
  }
  name = ID2SYM(rb_intern(str_name));

  missing_calls_count = rb_iv_get(self, "@method_missing_calls_count");
  count = rb_hash_aref(missing_calls_count, name);
  if (NIL_P(count)) {
    count = INT2FIX(0);
  }
  count = INT2NUM(NUM2INT(count) + 1);
  rb_hash_aset(missing_calls_count, name, count);

  calls_until_define_accessor = rb_iv_get(cFOpenStruct, "@calls_until_define_accessor");

  if (!NIL_P(calls_until_define_accessor) && NUM2INT(calls_until_define_accessor) >= NUM2INT(count)) {
    define_variable_accessor(self, name);
  }
}

static VALUE fos_method_missing(int argc, VALUE* argv, VALUE self) {
  VALUE name;

  name = argv[0];

  if (argc == 1) {
    VALUE ivar;
    ID ivar_id;

    ivar = fos_ivar_for_name(self, name);
    ivar_id = SYM2ID(ivar);

    increase_missing_calls_count(self, ivar);

    if (rb_ivar_defined(self, ivar_id)) {
      return rb_ivar_get(self, ivar_id);
    }
    return Qnil;
  } else {
    const char* str_name = rb_id2name(SYM2ID(name));
    if (argc == 2 && str_name[strlen(str_name) - 1] == '=') {
      VALUE ivar;
      ID ivar_id;

      ivar = fos_ivar_for_setter(self, name);
      ivar_id = SYM2ID(ivar);

      increase_missing_calls_count(self, ivar);

      return rb_ivar_set(self, ivar_id, argv[1]);
    } else {
      return rb_call_super(argc, argv);
    }
  }
}

static VALUE fos_initialize(int argc, VALUE* argv, VALUE self) {
  VALUE table;

  if (argc > 1) {
    rb_raise(rb_eArgError, "wrong number of arguments");
  }

  if (argc == 0) {
    table = rb_hash_new();
  } else {
    table = argv[0];
  }

  rb_iv_set(self, "@method_missing_calls_count", rb_hash_new());

  rb_block_call(table, id_each_pair, 0, 0, set_table_variables_block, self);

  return self;
}

void Init_fopen_struct() {
  rb_require("set");
  cSet = rb_const_get(rb_cObject, rb_intern("Set"));

  id_puts = rb_intern("puts");
  id_each_pair = rb_intern("each_pair");
  id_to_enum = rb_intern("to_enum");
  id_to_a = rb_intern("to_a");
  id_battery = rb_intern("[]");
  id_hash = rb_intern("hash");
  id_map = rb_intern("map");
  id_sort = rb_intern("sort");
  id_default = rb_intern("default");
  id_ivar_for_name = rb_intern("ivar_for_name");
  id_ivar_for_setter = rb_intern("ivar_for_setter");

  cFOpenStruct = rb_define_class("FOpenStruct", rb_cObject);
  cFOpenStructSingleton = rb_singleton_class(cFOpenStruct);

  rb_define_alias(cFOpenStructSingleton, "old_new", "new");
  rb_define_method(cFOpenStructSingleton, "create_class", fos_class_create_class, 1);
  rb_define_method(cFOpenStructSingleton, "new", fos_class_new, -1);

  rb_define_attr(cFOpenStructSingleton, "calls_until_define_accessor", 1, 1);
  rb_iv_set(cFOpenStruct, "@calls_until_define_accessor", Qnil);

  rb_iv_set(cFOpenStruct, "@cache", rb_hash_new());
  rb_cv_set(cFOpenStruct, "@@ivar_for_names", rb_hash_new());
  rb_cv_set(cFOpenStruct, "@@ivar_for_setters", rb_hash_new());

  rb_define_method(cFOpenStruct, "initialize", fos_initialize, -1);
  rb_define_private_method(cFOpenStruct, "ivar_for_name", fos_ivar_for_name, 1);
  rb_define_private_method(cFOpenStruct, "ivar_for_setter", fos_ivar_for_setter, 1);
  rb_define_private_method(cFOpenStruct, "apparent_class", fos_apparent_class, 0);
  rb_define_method(cFOpenStruct, "get", fos_get, 1);
  rb_define_method(cFOpenStruct, "set", fos_set, 2);
  rb_define_alias(cFOpenStruct, "[]", "get");
  rb_define_alias(cFOpenStruct, "[]=", "set");
  rb_define_method(cFOpenStruct, "delete_field", fos_delete_field, 1);
  rb_define_method(cFOpenStruct, "each_pair", fos_each_pair, 0);
  rb_define_method(cFOpenStruct, "==", fos_equal, 1);
  rb_define_method(cFOpenStruct, "eql?", fos_eql, 1);
  // rb_define_method(cFOpenStruct, "inspect", fos_inspect, 0);
  rb_define_method(cFOpenStruct, "to_a", fos_to_a, 0);
  rb_define_method(cFOpenStruct, "to_h", fos_to_h, 0);
  rb_define_method(cFOpenStruct, "hash", fos_hash, 0);
  rb_define_method(cFOpenStruct, "respond_to?", fos_respond_to_p, 1);
  rb_define_method(cFOpenStruct, "method_missing", fos_method_missing, -1);
}