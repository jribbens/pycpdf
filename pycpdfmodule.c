#define PY_SSIZE_T_CLEAN
#include <stddef.h>
#include "Python.h"
#include "structmember.h"

#if PY_MAJOR_VERSION >= 3
#define FORMAT_BYTES "y#"
#else
#define FORMAT_BYTES "s#"
#endif


static PyObject *baseerror = NULL;
static PyObject *pdferror = NULL;
static PyObject *notsupportederror = NULL;
static PyObject *badpassworderror = NULL;
static PyObject *encodings;
static PyObject *pdfdocencoding;
static PyObject *glyphlist;
static PyObject *unicode_translations;
static PyObject *md5;
static PyObject *decompressobj;
static PyObject *string_page;
static PyObject *string_prev;
static PyObject *string_emptystring;
static PyObject *string_newline;
static PyObject *string_unknown;
static PyObject *bytestr_emptystring;
static PyObject *name_inlineimage;

static PyTypeObject PDFType;
static PyTypeObject IndirectObjectType;
static PyTypeObject ArrayType;
typedef struct IndirectObject IndirectObject;
static PyObject *array_item(PyObject *self, Py_ssize_t i);
static PyObject *array_slice(PyObject *self, Py_ssize_t i1, Py_ssize_t i2);
static PyObject *array_subscript(PyObject *self, PyObject *key);
static PyObject *dictionary_subscript(PyObject *self, PyObject *key);
static PyObject *dictionary_get(PyObject *self, PyObject *args);
static PyObject *indirectobject_getObject(IndirectObject *self);


typedef struct {
  PyUnicodeObject unicode;
} Name;


static PyTypeObject NameType = {
  PyVarObject_HEAD_INIT(NULL, 0)
  "pycpdf.Name",              /*tp_name*/
  sizeof(Name),               /*tp_basicsize*/
  0,                          /*tp_itemsize*/
  0,                          /*tp_dealloc*/
  0,                          /*tp_print*/
  0,                          /*tp_getattr*/
  0,                          /*tp_setattr*/
  0,                          /*tp_compare*/
  0,                          /*tp_repr*/
  0,                          /*tp_as_number*/
  0,                          /*tp_as_sequence*/
  0,                          /*tp_as_mapping*/
  0,                          /*tp_hash*/
  0,                          /*tp_call*/
  0,                          /*tp_str*/
  0,                          /*tp_getattro*/
  0,                          /*tp_setattro*/
  0,                          /*tp_as_buffer*/
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
  "PDF Name objects",         /*tp_doc*/
  0,                          /*tp_traverse*/
  0,                          /*tp_clear*/
  0,                          /*tp_richcompare*/
  0,                          /*tp_weaklistoffset*/
  0,                          /*tp_iter*/
  0,                          /*tp_iternext*/
  0,                          /*tp_methods*/
  0,                          /*tp_members*/
  0,                          /*tp_getset*/
  0,                          /*tp_base*/
  0,                          /*tp_dict*/
  0,                          /*tp_descr_get*/
  0,                          /*tp_descr_set*/
  0,                          /*tp_dictoffset*/
  0,                          /*tp_init*/
  0,                          /*tp_alloc*/
  0,                          /*tp_new*/
  0,                          /*tp_free*/
  0,                          /*tp_is_gc*/
};


typedef struct {
  PyUnicodeObject unicode;
} Operator;


static PyTypeObject OperatorType = {
  PyVarObject_HEAD_INIT(NULL, 0)
  "pycpdf.Operator",          /*tp_name*/
  sizeof(Operator),           /*tp_basicsize*/
  0,                          /*tp_itemsize*/
  0,                          /*tp_dealloc*/
  0,                          /*tp_print*/
  0,                          /*tp_getattr*/
  0,                          /*tp_setattr*/
  0,                          /*tp_compare*/
  0,                          /*tp_repr*/
  0,                          /*tp_as_number*/
  0,                          /*tp_as_sequence*/
  0,                          /*tp_as_mapping*/
  0,                          /*tp_hash*/
  0,                          /*tp_call*/
  0,                          /*tp_str*/
  0,                          /*tp_getattro*/
  0,                          /*tp_setattro*/
  0,                          /*tp_as_buffer*/
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
  "PDF Operator objects",     /*tp_doc*/
  0,                          /*tp_traverse*/
  0,                          /*tp_clear*/
  0,                          /*tp_richcompare*/
  0,                          /*tp_weaklistoffset*/
  0,                          /*tp_iter*/
  0,                          /*tp_iternext*/
  0,                          /*tp_methods*/
  0,                          /*tp_members*/
  0,                          /*tp_getset*/
  0,                          /*tp_base*/
  0,                          /*tp_dict*/
  0,                          /*tp_descr_get*/
  0,                          /*tp_descr_set*/
  0,                          /*tp_dictoffset*/
  0,                          /*tp_init*/
  0,                          /*tp_alloc*/
  0,                          /*tp_new*/
  0,                          /*tp_free*/
  0,                          /*tp_is_gc*/
};


typedef struct {
  PyUnicodeObject unicode;
  PyObject *raw;
} PDFString;


static PyObject *pdfstring_new(PyTypeObject *type, PyObject *args,
    PyObject *kwargs) {
  PDFString *self;

  self = (PDFString *)PyUnicode_Type.tp_new(type, args, kwargs);
  if (self)
    self->raw = NULL;
  return (PyObject *)self;
}


static void pdfstring_dealloc(PDFString *self) {
  Py_CLEAR(self->raw);
  PyUnicode_Type.tp_dealloc((PyObject *)self);
}


static PyMemberDef pdfstring_members[] = {
  { "raw_bytes", T_OBJECT, offsetof(PDFString, raw), READONLY,
    "The original bytestream from the PDF file" },
  { NULL }
};


static PyTypeObject PDFStringType = {
  PyVarObject_HEAD_INIT(NULL, 0)
  "pycpdf.PDFString",         /*tp_name*/
  sizeof(PDFString),          /*tp_basicsize*/
  0,                          /*tp_itemsize*/
  (destructor)pdfstring_dealloc, /*tp_dealloc*/
  0,                          /*tp_print*/
  0,                          /*tp_getattr*/
  0,                          /*tp_setattr*/
  0,                          /*tp_compare*/
  0,                          /*tp_repr*/
  0,                          /*tp_as_number*/
  0,                          /*tp_as_sequence*/
  0,                          /*tp_as_mapping*/
  0,                          /*tp_hash*/
  0,                          /*tp_call*/
  0,                          /*tp_str*/
  0,                          /*tp_getattro*/
  0,                          /*tp_setattro*/
  0,                          /*tp_as_buffer*/
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
  "PDF String objects",       /*tp_doc*/
  0,                          /*tp_traverse*/
  0,                          /*tp_clear*/
  0,                          /*tp_richcompare*/
  0,                          /*tp_weaklistoffset*/
  0,                          /*tp_iter*/
  0,                          /*tp_iternext*/
  0,                          /*tp_methods*/
  pdfstring_members,          /*tp_members*/
  0,                          /*tp_getset*/
  0,                          /*tp_base*/
  0,                          /*tp_dict*/
  0,                          /*tp_descr_get*/
  0,                          /*tp_descr_set*/
  0,                          /*tp_dictoffset*/
  0,                          /*tp_init*/
  0,                          /*tp_alloc*/
  pdfstring_new,              /*tp_new*/
  0,                          /*tp_free*/
  0,                          /*tp_is_gc*/
};


static int md5update(PyObject *md5obj, const char *data, Py_ssize_t length) {
  PyObject *obj;

  if (!(obj = PyObject_CallMethod(md5obj, "update", FORMAT_BYTES, data,
          length)))
    return 0;
  Py_DECREF(obj);
  return 1;
}


static int md5update_str(PyObject *md5obj, PyObject *obj, int maxlen) {
  PyObject *ret;

  if (obj->ob_type == &PDFStringType) {
    obj = ((PDFString *)obj)->raw;
  } else if (!PyBytes_Check(obj)) {
    PyErr_SetString(pdferror, "MD5 update value is not a string");
    return 0;
  }
  if (maxlen) {
    PyObject *slice;
    if (!(slice = PySequence_GetSlice(obj, 0, maxlen)))
      return 0;
    ret = PyObject_CallMethod(md5obj, "update", "(O)", slice);
    Py_DECREF(slice);
  } else {
    ret = PyObject_CallMethod(md5obj, "update", "(O)", obj);
  }
  if (!ret)
    return 0;
  Py_DECREF(ret);
  return 1;
}


static void rc4(const char *key, long keylen, char *data, long datalen) {
   int i, j, t;
   unsigned char state[256];
   unsigned char *cp;

   for (i = 0; i < 256; ++i)
      state[i] = i;
   j = 0;
   for (i = 0; i < 256; ++i) {
      j = (j + state[i] + ((const unsigned char *)key)[i % keylen]) % 256;
      t = state[i];
      state[i] = state[j];
      state[j] = t;
   }

   i = j = 0;
   for (cp = (unsigned char *)data; cp < ((unsigned char *)data) + datalen;
       cp++) {
      i = (i + 1) % 256;
      j = (j + state[i]) % 256;
      t = state[i];
      state[i] = state[j];
      state[j] = t;
      *cp ^= state[(state[i] + state[j]) % 256];
   }
}


static int is_pdfwhitespace(char c) {
  return c == '\x09' || c == '\x0a' || c == '\x0c' || c == '\x0d' || c == ' ';
}


static int is_pdfdelimiter(char c) {
  return c == '(' || c == ')' || c == '<' || c == '>' || c == '[' ||
    c == ']' || c == '{' || c == '}' || c == '/' || c == '%';
}


static const char *skipwhitespace(const char *cp, const char *end) {
  while (cp < end) {
    if (*cp == '%') {
      cp++;
      while (cp < end && *cp != '\r' && *cp != '\n')
        cp++;
    } else if (is_pdfwhitespace(*cp)) {
      cp++;
    } else {
      return cp;
    }
  }
  return cp;
}


static const char *previous_line(const char *pos, const char **eol,
    const char *start) {
  const char *cp = pos - 1;
  const char *end;

  while (cp > start && *cp != '\r' && *cp != '\n')
    cp--;
  while (cp > start && is_pdfwhitespace(*cp))
    cp--;
  if (cp <= start || cp >= pos - 1)
    return NULL;
  end = cp + 1;
  while (cp > start && *cp != '\r' && *cp != '\n')
    cp--;
  cp++;
  while (cp < end && is_pdfwhitespace(*cp))
    cp++;
  if (cp >= end)
    return NULL;
  if (eol)
    *eol = end;
  return cp;
}


static int read_integer(const char **start, const char *end, long *retval) {
  const char *cp = *start;
  char digits[32];
  size_t len;

  if (cp >= end)
    return 0;
  len = 0;
  if (*cp == '+' || *cp == '-')
    digits[len++] = *(cp++);
  while (cp < end && *cp >= '0' && *cp <= '9') {
    if (len >= sizeof(digits) - 1)
      return 0;
    digits[len++] = *(cp++);
  }
  if (cp < end && !is_pdfwhitespace(*cp) && !is_pdfdelimiter(*cp))
    return 0;
  digits[len] = 0;

  errno = 0;
  *retval = strtol(digits, (char **)&cp, 10);
  if (errno || *cp)
    return 0;
  *start += cp - digits;
  return 1;
}


static int read_twointegers(const char **start, const char *end, long *num1p,
    long *num2p, const char *keyword) {
  const char *cp;
  int keylen = keyword ? strlen(keyword) : 0;

  cp = skipwhitespace(*start, end);
  if (cp >= end || *cp < '0' || *cp > '9')
    return 0;
  while (cp < end && *cp >= '0' && *cp <= '9')
    cp++;
  cp = skipwhitespace(cp, end);
  if (cp >= end || *cp < '0' || *cp > '9')
    return 0;
  while (cp < end && *cp >= '0' && *cp <= '9')
    cp++;
  cp = skipwhitespace(cp, end);
  if (keyword) {
    if (cp + keylen > end || memcmp(cp, keyword, keylen))
      return 0;
    if (cp + keylen < end) {
      if (!is_pdfwhitespace(cp[keylen]) && !is_pdfdelimiter(cp[keylen]))
        return 0;
    }
  }
  cp = skipwhitespace(*start, end);
  if (!read_integer(&cp, end, num1p))
    return 0;
  cp = skipwhitespace(cp, end);
  if (!read_integer(&cp, end, num2p))
    return 0;
  *start = skipwhitespace(cp, end) + keylen;
  return 1;
}


static int read_float(const char **start, const char *end, double *retval) {
  const char *cp = *start;
  char digits[64];
  size_t len;

  for (len = 0; len < sizeof(digits) - 1 && cp < end &&
      ((*cp >= '0' && *cp <= '9') || *cp == '+' || *cp == '-' || *cp == '.');
      len++)
    digits[len] = *(cp++);
  if (cp < end && !is_pdfwhitespace(*cp) && !is_pdfdelimiter(*cp))
    return 0;
  digits[len] = 0;

  errno = 0;
  *retval = strtod(digits, (char **)&cp);
  if (errno || *cp)
    return 0;
  *start += cp - digits;
  return 1;
}


static char decodexdigit(char c) {
  if (c >= 'a' && c <= 'f')
    return 10 + (c - 'a');
  if (c >= 'A' && c <= 'F')
    return 10 + (c - 'A');
  if (c >= '0' && c <= '9')
    return c - '0';
  return 16;
}


typedef struct {
  PyListObject list;
} Array;


static PyMappingMethods array_as_mapping = {
  NULL,                       /*mp_length*/
  array_subscript,            /*mp_subscript*/
  NULL,                       /*mp_ass_subscript*/
};


static PySequenceMethods array_as_sequence = {
  NULL,                       /*sq_length*/
  NULL,                       /*sq_concat*/
  NULL,                       /*sq_repeat*/
  (ssizeargfunc)array_item,   /*sq_item*/
  (ssizessizeargfunc)array_slice, /*sq_slice*/
  NULL,                       /*sq_ass_slice*/
  NULL,                       /*sq_ass_item*/
  NULL,                       /*sq_contains*/
  NULL,                       /*sq_inplace_concat*/
  NULL,                       /*sq_inplace_repeat*/
};


static PyMethodDef array_methods[] = {
  /*{ "get", (PyCFunction)dictionary_get, METH_VARARGS,
    "D.get(k[,d]) -> D[k] if k in D, else d.  d defaults to None." },*/
  { NULL }
};


static PyObject *array_iter(PyObject *self);


static PyTypeObject ArrayType = {
  PyVarObject_HEAD_INIT(NULL, 0)
  "pycpdf.Array",             /*tp_name*/
  sizeof(Array),              /*tp_basicsize*/
  0,                          /*tp_itemsize*/
  0,                          /*tp_dealloc*/
  0,                          /*tp_print*/
  0,                          /*tp_getattr*/
  0,                          /*tp_setattr*/
  0,                          /*tp_compare*/
  0,                          /*tp_repr*/
  0,                          /*tp_as_number*/
  &array_as_sequence,         /*tp_as_sequence*/
  &array_as_mapping,          /*tp_as_mapping*/
  0,                          /*tp_hash*/
  0,                          /*tp_call*/
  0,                          /*tp_str*/
  0,                          /*tp_getattro*/
  0,                          /*tp_setattro*/
  0,                          /*tp_as_buffer*/
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_GC, /*tp_flags*/
  "PDF Array objects",        /*tp_doc*/
  0,                          /*tp_traverse*/
  0,                          /*tp_clear*/
  0,                          /*tp_richcompare*/
  0,                          /*tp_weaklistoffset*/
  array_iter,                 /*tp_iter*/
  0,                          /*tp_iternext*/
  array_methods,              /*tp_methods*/
  0,                          /*tp_members*/
  0,                          /*tp_getset*/
  0,                          /*tp_base*/
  0,                          /*tp_dict*/
  0,                          /*tp_descr_get*/
  0,                          /*tp_descr_set*/
  0,                          /*tp_dictoffset*/
  0,                          /*tp_init*/
  0,                          /*tp_alloc*/
  0,                          /*tp_new*/
  0,                          /*tp_free*/
  0,                          /*tp_is_gc*/
};


static PyObject *array_item(PyObject *self, Py_ssize_t i) {
  PyObject *obj;

  if (!(obj = PyList_GetItem(self, i)))
    return NULL;
  if (obj->ob_type == &IndirectObjectType) {
    if (!(obj = indirectobject_getObject((IndirectObject *)obj)))
      return NULL;
    if (PyList_SetItem(self, i, obj) < 0) {
      Py_DECREF(obj);
      return NULL;
    }
  }
  Py_INCREF(obj);
  return obj;
}


static PyObject *array_slice(PyObject *self, Py_ssize_t i1, Py_ssize_t i2) {
  PyObject *result;
  Py_ssize_t i;
  Py_ssize_t len;

  len = PyList_Size(self);
  if (i1 < 0)
    i1 = 0;
  else if (i1 > len)
    i1 = len;
  if (i2 < i1)
    i2 = i1;
  else if (i2 > len)
    i2 = len;

  if (!(result = PyList_New(i2 - i1)))
    return NULL;
  for (i = 0; i < i2 - i1; i++) {
    PyObject *obj;
    if (!(obj = array_item(self, i1 + i))) {
      Py_DECREF(result);
      return NULL;
    }
    PyList_SET_ITEM(result, i, obj);
  }
  return result;
}


static PyObject *array_subscript(PyObject *self, PyObject *key) {
  Py_ssize_t i;

  if (!PyIndex_Check(key))
    return PyErr_Format(PyExc_TypeError, "array index is a %s not an integer",
        key->ob_type->tp_name);
  if ((i = PyNumber_AsSsize_t(key, PyExc_IndexError)) == -1 &&
      PyErr_Occurred())
    return NULL;
  return array_item(self, i < 0 ? i + PyList_Size(self) : i);
}


typedef struct {
  PyObject_HEAD
  Py_ssize_t index;
  PyObject *array;
} ArrayIterator;


static void arrayiterator_clear(ArrayIterator *self) {
  Py_CLEAR(self->array);
}


static int arrayiterator_traverse(ArrayIterator *self, visitproc visit,
    void *arg) {
  Py_VISIT(self->array);
  return 0;
}


static void arrayiterator_dealloc(ArrayIterator *self) {
  arrayiterator_clear(self);
  ((PyObject *)self)->ob_type->tp_free((PyObject *)self);
}


static PyObject *arrayiterator_next(ArrayIterator *self) {
  PyObject *obj;
  Py_ssize_t len;

  if (!self->array)
    return NULL;
  if ((len = PySequence_Length(self->array)) < 0)
    return NULL;
  if (self->index >= len) {
    Py_CLEAR(self->array);
    return NULL;
  }
  if (!(obj = PySequence_GetItem(self->array, self->index)))
    return NULL;
  self->index++;
  return obj;
}


static PyTypeObject ArrayIteratorType = {
  PyVarObject_HEAD_INIT(NULL, 0)
  "pycpdf.ArrayIterator",     /*tp_name*/
  sizeof(ArrayIterator),      /*tp_basicsize*/
  0,                          /*tp_itemsize*/
  (destructor)arrayiterator_dealloc, /*tp_dealloc*/
  0,                          /*tp_print*/
  0,                          /*tp_getattr*/
  0,                          /*tp_setattr*/
  0,                          /*tp_compare*/
  0,                          /*tp_repr*/
  0,                          /*tp_as_number*/
  0,                          /*tp_as_sequence*/
  0,                          /*tp_as_mapping*/
  0,                          /*tp_hash*/
  0,                          /*tp_call*/
  0,                          /*tp_str*/
  0,                          /*tp_getattro*/
  0,                          /*tp_setattro*/
  0,                          /*tp_as_buffer*/
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC, /*tp_flags*/
  "Iterator over Array objects", /*tp_doc*/
  (traverseproc)arrayiterator_traverse, /*tp_traverse*/
  (inquiry)arrayiterator_clear, /*tp_clear*/
  0,                          /*tp_richcompare*/
  0,                          /*tp_weaklistoffset*/
  PyObject_SelfIter,          /*tp_iter*/
  (iternextfunc)arrayiterator_next, /*tp_iternext*/
  0,                          /*tp_methods*/
  0,                          /*tp_members*/
  0,                          /*tp_getset*/
  0,                          /*tp_base*/
  0,                          /*tp_dict*/
  0,                          /*tp_descr_get*/
  0,                          /*tp_descr_set*/
  0,                          /*tp_dictoffset*/
  0,                          /*tp_init*/
  0,                          /*tp_alloc*/
  0,                          /*tp_new*/
  0,                          /*tp_free*/
  0,                          /*tp_is_gc*/
};


static PyObject *array_iter(PyObject *self) {
  ArrayIterator *iter;

  if (!(iter = (ArrayIterator *)ArrayIteratorType.tp_alloc(
          &ArrayIteratorType, 0)))
    return NULL;
  iter->index = 0;
  Py_INCREF(self);
  iter->array = self;
  return (PyObject *)iter;
}


typedef struct {
  PyDictObject dict;
} Dictionary;


static PyMappingMethods dictionary_as_mapping = {
  NULL,                       /*mp_length*/
  dictionary_subscript,       /*mp_subscript*/
  NULL,                       /*mp_ass_subscript*/
};


static PyMethodDef dictionary_methods[] = {
  { "get", (PyCFunction)dictionary_get, METH_VARARGS,
    "D.get(k[,d]) -> D[k] if k in D, else d.  d defaults to None." },
  { NULL }
};


static PyTypeObject DictionaryType = {
  PyVarObject_HEAD_INIT(NULL, 0)
  "pycpdf.Dictionary",        /*tp_name*/
  sizeof(Dictionary),         /*tp_basicsize*/
  0,                          /*tp_itemsize*/
  0,                          /*tp_dealloc*/
  0,                          /*tp_print*/
  0,                          /*tp_getattr*/
  0,                          /*tp_setattr*/
  0,                          /*tp_compare*/
  0,                          /*tp_repr*/
  0,                          /*tp_as_number*/
  0,                          /*tp_as_sequence*/
  &dictionary_as_mapping,     /*tp_as_mapping*/
  0,                          /*tp_hash*/
  0,                          /*tp_call*/
  0,                          /*tp_str*/
  0,                          /*tp_getattro*/
  0,                          /*tp_setattro*/
  0,                          /*tp_as_buffer*/
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_GC, /*tp_flags*/
  "PDF Dictionary objects",   /*tp_doc*/
  0,                          /*tp_traverse*/
  0,                          /*tp_clear*/
  0,                          /*tp_richcompare*/
  0,                          /*tp_weaklistoffset*/
  0,                          /*tp_iter*/
  0,                          /*tp_iternext*/
  dictionary_methods,         /*tp_methods*/
  0,                          /*tp_members*/
  0,                          /*tp_getset*/
  0,                          /*tp_base*/
  0,                          /*tp_dict*/
  0,                          /*tp_descr_get*/
  0,                          /*tp_descr_set*/
  0,                          /*tp_dictoffset*/
  0,                          /*tp_init*/
  0,                          /*tp_alloc*/
  0,                          /*tp_new*/
  0,                          /*tp_free*/
  0,                          /*tp_is_gc*/
};


static PyObject *dictionary_subscript(PyObject *self, PyObject *key) {
  PyObject *obj;

  if (!(obj = PyDict_GetItem(self, key))) {
    PyObject *arg = PyTuple_Pack(1, key);
    if (arg) {
      PyErr_SetObject(PyExc_KeyError, arg);
      Py_DECREF(arg);
    }
    return NULL;
  }
  if (obj->ob_type == &IndirectObjectType) {
    if (!(obj = indirectobject_getObject((IndirectObject *)obj)))
      return NULL;
    if (PyDict_SetItem(self, key, obj) < 0)
      return NULL;
  } else {
    Py_INCREF(obj);
  }
  return obj;
}


static PyObject *dictionary_get(PyObject *self, PyObject *args) {
  PyObject *key;
  PyObject *failobj = Py_None;
  int check;

  if (!PyArg_UnpackTuple(args, "get", 1, 2, &key, &failobj))
    return NULL;
  if ((check = PyDict_Contains(self, key)) < 0)
    return NULL;
  if (!check) {
    Py_INCREF(failobj);
    return failobj;
  }
  return dictionary_subscript(self, key);
}


typedef struct {
  PyObject_HEAD
  PyObject *weakreflist;
  PyObject *source;
  PyObject *version;
  PyObject *xref;
  PyObject *trailer;
  PyObject *key;
  PyObject *catalog;
  PyObject *info;
  PyObject *linearized;
  PyObject *pages;
  Py_buffer data;
} PDF;


static PyObject *process_filter(PDF *self, PyObject *value, PyObject *name,
    PyObject *parms, long *usedbytes);
static PyObject *decode_string(const char *string, Py_ssize_t len,
    PyObject *decryption_key, char unicode);
static PyObject *parse_contents(PDF *self, PyObject *contents);


Py_GCC_ATTRIBUTE((format(printf, 3, 4)))
static PyObject *pdf_error(PDF *self, const char *cp, const char *format,
    ...) {
  char msgbuf[1000];
  va_list argptr;

  va_start(argptr, format);
  vsnprintf(msgbuf, sizeof(msgbuf) - 100, format, argptr);
  msgbuf[sizeof(msgbuf) - 100] = '\0';
  if (cp && self && self->data.buf && cp >= (const char *)self->data.buf &&
      cp < (((const char *)self->data.buf) + self->data.len))
    sprintf(msgbuf + strlen(msgbuf), " at file offset %ld",
        (long)(cp - (const char *)self->data.buf));
  PyErr_SetString(pdferror, msgbuf);
  return NULL;
}


static int get_intval(PyObject *dict, const char *key, long *retval) {
  PyObject *obj;

  if (!(obj = PyMapping_GetItemString(dict, (char *)key)))
    return 0;
  if (!PyLong_Check(obj)) {
    pdf_error(NULL, NULL, "Dictionary %s value is a %s not an integer", key,
        obj->ob_type->tp_name);
    Py_DECREF(obj);
    return 0;
  }
  *retval = PyLong_AsLong(obj);
  Py_DECREF(obj);
  return (*retval != -1 || !PyErr_Occurred());
}


typedef struct {
  Dictionary dict;
  PDF *pdf;
  PyObject *data;
  PyObject *contents;
  int decoded;
} StreamObject;


static int streamobject_clear(StreamObject *self) {
  Py_CLEAR(self->pdf);
  Py_CLEAR(self->data);
  Py_CLEAR(self->contents);
  return PyDict_Type.tp_clear((PyObject *)self);
}


static int streamobject_traverse(StreamObject *self, visitproc visit,
    void *arg) {
  Py_VISIT(self->pdf);
  Py_VISIT(self->data);
  Py_VISIT(self->contents);
  return PyDict_Type.tp_traverse((PyObject *)self, visit, arg);
}


static void streamobject_dealloc(StreamObject *self) {
  streamobject_clear(self);
  PyDict_Type.tp_dealloc((PyObject *)self);
}


static PyObject *streamobject_new(PyTypeObject *type, PyObject *args,
    PyObject *kwargs) {
  StreamObject *self;
  PDF *pdf;
  PyObject *dict;
  PyObject *data;
  PyObject *dictargs;

  if (!PyArg_ParseTuple(args, "O!OO:StreamObject", &PDFType, &pdf, &dict,
        &data))
    return NULL;
  if (!(dictargs = Py_BuildValue("(O)", dict)))
    return NULL;
  self = (StreamObject *)DictionaryType.tp_new(type, dictargs, kwargs);
  Py_DECREF(dictargs);
  if (!self)
    return NULL;
  self->contents = NULL;
  Py_INCREF(pdf);
  self->pdf = pdf;
  Py_INCREF(data);
  self->data = data;
  self->decoded = !PyMapping_HasKeyString(dict, "Filter");
  return (PyObject *)self;
}


static int streamobject_init(StreamObject *self, PyObject *args,
    PyObject *kwargs) {
  PDF *pdf;
  PyObject *dict;
  PyObject *data;
  PyObject *dictargs;
  int retval;

  if (!PyArg_ParseTuple(args, "O!OO:StreamObject", &PDFType, &pdf, &dict,
        &data))
    return -1;
  if (!(dictargs = Py_BuildValue("(O)", dict)))
    return -1;
  retval = DictionaryType.tp_init((PyObject *)self, dictargs, kwargs);
  Py_DECREF(dictargs);
  return retval;
}


static PyObject *read_filtered_data(PDF *self, PyObject *value,
    PyObject *filter, PyObject *parms, long *length) {
  PyObject *name;
  PyObject *result;
  PyObject *obj;
  PyObject *iterator;
  long len;

  if (value->ob_type == &PDFStringType)
    value = ((PDFString *)value)->raw;
  if (!PyBytes_Check(value))
    return pdf_error(self, NULL, "Filtered data is a %s not a string",
        value->ob_type->tp_name);
  if (!length)
    length = &len;
  if (filter->ob_type == &NameType)
    return process_filter(self, value, filter, parms, length);
  else if (filter->ob_type != &ArrayType)
    return pdf_error(self, NULL, "Stream Filter value is a %s"
        " not a pycpdf.Array or a pycpdf.Name", filter->ob_type->tp_name);
  if (!(iterator = PyObject_GetIter(filter)))
    return NULL;
  Py_INCREF(value);
  result = value;
  while ((name = PyIter_Next(iterator))) {
    if (name->ob_type != &NameType) {
      pdf_error(self, NULL,
          "Stream Filter array value is a %s not a pycpdf.Name",
          name->ob_type->tp_name);
      Py_DECREF(name);
      Py_DECREF(iterator);
      Py_DECREF(result);
      return NULL;
    }
    if (!(obj = process_filter(self, result, name, parms,
            result == value ? length : &len))) {
      Py_DECREF(name);
      Py_DECREF(result);
      Py_DECREF(iterator);
      return NULL;
    }
    Py_DECREF(result);
    result = obj;
    Py_DECREF(name);
  }
  Py_DECREF(iterator);
  if (PyErr_Occurred()) {
    Py_DECREF(result);
    return NULL;
  }
  return result;
}


static PyObject *streamobject_get_data(StreamObject *self, void *closure) {
  if (!self->decoded && PyMapping_HasKeyString((PyObject *)self, "Filter")) {
    PyObject *name;
    PyObject *parms;
    PyObject *obj;
    PyObject *value;
    char *buffer;
    Py_ssize_t buflen;

    if (!(name = PyMapping_GetItemString((PyObject *)self, "Filter")))
      return NULL;
    if (PyMapping_HasKeyString((PyObject *)self, "DecodeParms")) {
      if (!(parms = PyMapping_GetItemString(
              (PyObject *)self, "DecodeParms"))) {
        Py_DECREF(name);
        return NULL;
      }
    } else {
      parms = Py_None;
      Py_INCREF(Py_None);
    }
    value = read_filtered_data(self->pdf, self->data, name, parms, NULL);
    Py_DECREF(parms);
    Py_DECREF(name);
    if (!value)
      return NULL;
    if (PyBytes_AsStringAndSize(value, &buffer, &buflen) < 0) {
      Py_DECREF(value);
      return NULL;
    }
    obj = decode_string(buffer, buflen, NULL, 0);
    Py_DECREF(value);
    if (!obj)
      return NULL;
    Py_DECREF(self->data);
    self->data = obj;
    self->decoded = 1;
  }
  Py_INCREF(self->data);
  return self->data;
}


static PyObject *streamobject_get_contents(StreamObject *self,
    void *closure) {
  PyObject *data;
  PyObject *contents;

  if (self->contents) {
    Py_INCREF(self->contents);
    return self->contents;
  }

  if (!(data = streamobject_get_data(self, NULL)))
    return NULL;
  contents = parse_contents(self->pdf, data);
  Py_DECREF(data);
  if (!contents)
    return NULL;
  Py_INCREF(self->contents);
  self->contents = contents;
  return contents;
}


static PyGetSetDef streamobject_getset[] = {
  { "contents", (getter)streamobject_get_contents, NULL,
    "The sequence of operations that are contained in the stream data.",
    NULL },
  { "data", (getter)streamobject_get_data, NULL,
    "The data contained within the stream", NULL },
  { NULL }
};


static PyTypeObject StreamObjectType = {
  PyVarObject_HEAD_INIT(NULL, 0)
  "pycpdf.StreamObject",      /*tp_name*/
  sizeof(StreamObject),       /*tp_basicsize*/
  0,                          /*tp_itemsize*/
  (destructor)streamobject_dealloc, /*tp_dealloc*/
  0,                          /*tp_print*/
  0,                          /*tp_getattr*/
  0,                          /*tp_setattr*/
  0,                          /*tp_compare*/
  0,                          /*tp_repr*/
  0,                          /*tp_as_number*/
  0,                          /*tp_as_sequence*/
  0,                          /*tp_as_mapping*/
  0,                          /*tp_hash*/
  0,                          /*tp_call*/
  0,                          /*tp_str*/
  0,                          /*tp_getattro*/
  0,                          /*tp_setattro*/
  0,                          /*tp_as_buffer*/
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_GC, /*tp_flags*/
  "PDF Stream objects",       /*tp_doc*/
  (traverseproc)streamobject_traverse, /*tp_traverse*/
  (inquiry)streamobject_clear, /*tp_clear*/
  0,                          /*tp_richcompare*/
  0,                          /*tp_weaklistoffset*/
  0,                          /*tp_iter*/
  0,                          /*tp_iternext*/
  0,                          /*tp_methods*/
  0,                          /*tp_members*/
  streamobject_getset,        /*tp_getset*/
  0,                          /*tp_base*/
  0,                          /*tp_dict*/
  0,                          /*tp_descr_get*/
  0,                          /*tp_descr_set*/
  0,                          /*tp_dictoffset*/
  (initproc)streamobject_init, /*tp_init*/
  0,                          /*tp_alloc*/
  streamobject_new,           /*tp_new*/
  0,                          /*tp_free*/
  0,                          /*tp_is_gc*/
};


static PyObject *read_object(PDF *self, const char **start, const char *end,
    PyObject *decryption_key);


typedef struct {
  Dictionary dict;
  PyObject *contents;
  PyObject *text;
} Page;


static PyObject *parse_contents(PDF *self, PyObject *contents) {
  PyObject *result;
  char *data;
  Py_ssize_t len;
  const char *cp;
  char *end;
  PyObject *entry;

  if (PyBytes_AsStringAndSize((contents->ob_type == &PDFStringType) ?
        ((PDFString *)contents)->raw : contents, &data, &len) < 0)
    return NULL;
  if (!(result = PyList_New(0)))
    return NULL;
  cp = data;
  end = data + len;
  entry = NULL;
  while (cp < end) {
    PyObject *obj;
    cp = skipwhitespace(cp, end);
    if (cp >= end)
      break;
    if (!(obj = read_object(self, &cp, end, NULL))) {
      Py_XDECREF(entry);
      Py_DECREF(result);
      return NULL;
    }
    if (!entry) {
      if (!(entry = PyList_New(0))) {
        Py_DECREF(obj);
        Py_DECREF(result);
        return NULL;
      }
    }
    if (PyList_Append(entry, obj) < 0) {
      Py_DECREF(obj);
      Py_DECREF(entry);
      Py_DECREF(result);
      return NULL;
    }
    if (obj->ob_type == &OperatorType) {
      if (PyList_Append(result, entry) < 0) {
        Py_DECREF(obj);
        Py_DECREF(entry);
        Py_DECREF(result);
        return NULL;
      }
      Py_CLEAR(entry);
    }
    Py_DECREF(obj);
  }
  Py_CLEAR(entry);
  return result;
}


static PyObject *page_get_contents(Page *self, void *closure) {
  PDF *pdf = NULL;
  PyObject *result;
  PyObject *contents;
  PyObject *iter;
  PyObject *obj;
  PyObject *data;

  if (self->contents) {
    Py_INCREF(self->contents);
    return self->contents;
  }

  if (!PyMapping_HasKeyString((PyObject *)self, "Contents"))
    return PyList_New(0); /* no contents */
  if (!(obj = PyMapping_GetItemString((PyObject *)self, "Contents")))
    return NULL;
  if (!(contents = PyList_New(0))) {
    Py_DECREF(obj);
    return NULL;
  }
  if (obj->ob_type == &StreamObjectType) { /* single content stream */
    pdf = ((StreamObject *)obj)->pdf;
    data = streamobject_get_data((StreamObject *)obj, NULL);
    Py_DECREF(obj);
    if (!data) {
      Py_DECREF(contents);
      return NULL;
    }
    if (PyList_Append(contents, (data->ob_type == &PDFStringType) ?
        ((PDFString *)data)->raw : data) < 0) {
      Py_DECREF(data);
      Py_DECREF(contents);
      return NULL;
    }
    Py_DECREF(data);
  } else if (obj->ob_type == &ArrayType) { /* array of content streams */
    iter = PyObject_GetIter(obj);
    Py_DECREF(obj);
    if (!iter) {
      Py_DECREF(contents);
      return NULL;
    }
    while ((obj = PyIter_Next(iter))) {
      if (obj->ob_type != &StreamObjectType) {
        pdf_error(NULL, NULL,
            "Page Contents array value is a %s not a StreamObject",
            obj->ob_type->tp_name);
        Py_DECREF(obj);
        Py_DECREF(iter);
        Py_DECREF(contents);
        return NULL;
      }
      if (!pdf)
        pdf = ((StreamObject *)obj)->pdf;
      data = streamobject_get_data((StreamObject *)obj, NULL);
      Py_DECREF(obj);
      if (!data) {
        Py_DECREF(iter);
        Py_DECREF(contents);
        return NULL;
      }
      if (PyList_Append(contents, (data->ob_type == &PDFStringType) ?
          ((PDFString *)data)->raw : data) < 0) {
        Py_DECREF(data);
        Py_DECREF(iter);
        Py_DECREF(contents);
        return NULL;
      }
      Py_DECREF(data);
    }
    Py_DECREF(iter);
    if (PyErr_Occurred()) {
      Py_DECREF(contents);
      return NULL;
    }
  } else {
    pdf_error(NULL, NULL,
        "Page Contents value is a %s not a StreamObject or an Array",
        obj->ob_type->tp_name);
    Py_DECREF(obj);
    Py_DECREF(contents);
    return NULL;
  }
  obj = PyObject_CallMethod(bytestr_emptystring, "join", "O", contents);
  Py_DECREF(contents);
  if (!obj)
    return NULL;
  result = parse_contents(pdf, obj);
  Py_DECREF(obj);
  if (result) {
    Py_INCREF(result);
    self->contents = result;
  }
  return result;
}


static PyObject *decode_font_string_encoding(PyObject *encoding,
    const char *data, Py_ssize_t len) {
  PyObject *obj = NULL;
  PyObject *differences = NULL;
  PyObject *baseencoding = NULL;
  PyObject *iter = NULL;
  PyObject *item = NULL;
  PyObject *out = NULL;
  const char *end = data + len;
  long charno;
  long cmp;
  int found;

  if (PyMapping_HasKeyString(encoding, "BaseEncoding")) {
    if (!(obj = PyMapping_GetItemString(encoding, "BaseEncoding")))
      return NULL;
    if (PyMapping_HasKey(encodings, obj)) {
      if (!(baseencoding = PyObject_GetItem(encodings, obj))) {
        Py_DECREF(obj);
        return NULL;
      }
    }
    Py_CLEAR(obj);
  }
  if (!baseencoding) {
    if (!(baseencoding = PyMapping_GetItemString(encodings,
            "StandardEncoding")))
      return NULL;
  }
  if (PyMapping_HasKeyString(encoding, "Differences")) {
    if (!(differences = PyMapping_GetItemString(encoding, "Differences")))
      goto error;
  }
  if (!differences || differences->ob_type != &ArrayType) {
    /* no Differences, just use DecodeCharmap on the base encoding */
    obj = PyUnicode_DecodeCharmap(data, len, baseencoding, "ignore");
    Py_XDECREF(differences);
    Py_XDECREF(baseencoding);
    return obj;
  }
  if (!(out = PyList_New(0)))
    goto error;
  while (data < end) {
    charno = -1;
    found = 0;
    if (!(iter = PyObject_GetIter(differences)))
      goto error;
    while ((item = PyIter_Next(iter))) {
      if (PyLong_Check(item)) {
        charno = PyLong_AS_LONG(item);
      } else if (PyBytes_Check(item) || PyUnicode_Check(item)) {
        if (charno == *data) { /* found an entry for our byte value */
          if (!(obj = PyObject_GetItem(glyphlist, item))) {
            PyErr_Clear();
          } else {
            if (PyList_Append(out, obj) < 0)
              goto error;
            Py_CLEAR(obj);
          }
          Py_CLEAR(item);
          found = 1;
          break;
        } else {
          charno++;
        }
      }
      Py_CLEAR(item);
    }
    Py_CLEAR(iter);
    if (PyErr_Occurred())
      goto error;
    if (!found) {
      /* not found in Differences, fall back to base encoding */
      if (!(obj = PySequence_GetItem(baseencoding, *data)))
        goto error;
      cmp = PyObject_RichCompareBool(obj, string_unknown, Py_EQ);
      if (cmp < 0)
        goto error;
      if (!cmp && PyList_Append(out, obj) < 0)
        goto error;
      Py_CLEAR(obj);
    }
    data++;
  }
  obj = PyObject_CallMethod(string_emptystring, "join", "O", out);
  Py_XDECREF(out);
  Py_XDECREF(differences);
  Py_XDECREF(baseencoding);
  return obj;
error:
  Py_XDECREF(out);
  Py_XDECREF(item);
  Py_XDECREF(iter);
  Py_XDECREF(obj);
  Py_XDECREF(differences);
  Py_XDECREF(baseencoding);
  return NULL;
}


#if 0
static int cmap_match_coderange(PyObject *ranges, const unsigned char *data,
    Py_ssize_t len) {
  PyObject *obj;
  unsigned char *codebytes;
  Py_ssize_t rangeslen;
  Py_ssize_t codelen;
  Py_ssize_t codelen2;
  Py_ssize_t i;

  if ((rangeslen = PySequence_Length(ranges)) < 0)
    return -1;
  for (i = 0; i + 1 < rangeslen - 1; i += 2) {
    if (!(obj = PySequence_GetItem(ranges, i)))
      return -1;
    if (PyBytes_AsStringAndSize((obj->ob_type == &PDFStringType) ?
          ((PDFString *)obj)->raw : obj, (char **)&codebytes, &codelen) < 0) {
      Py_DECREF(obj);
      return -1;
    }
    Py_DECREF(obj);
    if (codelen < 1 || codelen > 3 || codelen > len) {
      /* code is invalid length, or longer than the remaining data */
      continue;
    }
    if (codelen == 1) {
      if (data[0] < codebytes[0])
        continue;
    } else if (codelen == 2) {
      if (((data[0] << 8) | data[1]) <
          ((codebytes[0] << 8) | codebytes[1]))
        continue;
    } else {
      if (((data[0] << 16) | (data[1] << 8) | data[0]) <
          ((codebytes[0] << 8) | (codebytes[1] << 8) | codebytes[0]))
        continue;
    }
    if (!(obj = PySequence_GetItem(ranges, i + 1)))
      return -1;
    if (PyBytes_AsStringAndSize((obj->ob_type == &PDFStringType) ?
          ((PDFString *)obj)->raw : obj, (char **)&codebytes,
          &codelen2) < 0) {
      Py_DECREF(obj);
      return -1;
    }
    Py_DECREF(obj);
    if (codelen2 != codelen)
      continue;
    if (codelen == 1) {
      if (data[0] > codebytes[0])
        continue;
    } else if (codelen == 2) {
      if (((data[0] << 8) | data[1]) >
          ((codebytes[0] << 8) | codebytes[1]))
        continue;
    } else {
      if (((data[0] << 16) | (data[1] << 8) | data[2]) >
          ((codebytes[0] << 16) | (codebytes[1] << 8) | codebytes[2]))
        continue;
    }
    return codelen;
  }
  return 0;
}
#endif


static PyObject *cmap_match_bfrange(PyObject *chars,
    const unsigned char *data, Py_ssize_t *lenp) {
  unsigned char *codebytes;
  Py_ssize_t charslen;
  Py_ssize_t codelen;
  Py_ssize_t i;
  Py_ssize_t valuelen;
  int byteorder;
  long offset;
  unsigned char resultbytes[2];

  if ((charslen = PySequence_Length(chars)) < 0)
    return NULL;
  for (i = 0; i + 2 < charslen - 1; i += 3) {
    PyObject *obj;
    if (!(obj = PySequence_GetItem(chars, i + 1)))
      return NULL;
    if (PyBytes_AsStringAndSize((obj->ob_type == &PDFStringType) ?
          ((PDFString *)obj)->raw : obj, (char **)&codebytes, &codelen) < 0) {
      Py_DECREF(obj);
      return NULL;
    }
    Py_DECREF(obj);
    if (codelen > *lenp)
      continue;
    if (codelen == 1) {
      if (data[0] > codebytes[0])
        continue;
    } else if (codelen == 2) {
      if (((data[0] << 8) | data[1]) >
          ((codebytes[0] << 8) | codebytes[1]))
        continue;
    } else {
      if (((data[0] << 16) | (data[1] << 8) | data[2]) >
          ((codebytes[0] << 16) | (codebytes[1] << 8) | codebytes[2]))
        continue;
    }
    if (!(obj = PySequence_GetItem(chars, i)))
      return NULL;
    if (PyBytes_AsStringAndSize((obj->ob_type == &PDFStringType) ?
          ((PDFString *)obj)->raw : obj, (char **)&codebytes,
          &valuelen) < 0) {
      Py_DECREF(obj);
      return NULL;
    }
    Py_DECREF(obj);
    if (valuelen != codelen)
      continue;
    if (codelen == 1) {
      offset = data[0] - codebytes[0];
    } else if (codelen == 2) {
      offset = ((data[0] << 8) | data[1]) -
          ((codebytes[0] << 8) | codebytes[1]);
    } else {
      offset = ((data[0] << 16) | (data[1] << 8) | data[2]) -
          ((codebytes[0] << 16) | (codebytes[1] << 8) | codebytes[2]);
    }
    if (offset < 0)
      continue;
    if (!(obj = PySequence_GetItem(chars, i + 2)))
      return NULL;
    if (obj->ob_type == &ArrayType) {
      PyObject *obj2 = PySequence_GetItem(obj, offset);
      Py_DECREF(obj);
      if (!obj2) {
        PyErr_Clear();
        continue;
      }
      if (obj2->ob_type == &NameType) {
        obj = PyObject_GetItem(glyphlist, obj2);
        Py_DECREF(obj2);
        if (!obj) {
          PyErr_Clear();
          continue;
        }
        *lenp = codelen;
        return obj;
      }
      if (obj2->ob_type != &PDFStringType && obj2->ob_type != &PyBytes_Type) {
        Py_DECREF(obj2);
        continue;
      }
      if (PyBytes_AsStringAndSize((obj2->ob_type == &PDFStringType) ?
            ((PDFString *)obj2)->raw : obj2, (char **)&codebytes, &valuelen)
          < 0) {
        Py_DECREF(obj2);
        return NULL;
      }
      Py_DECREF(obj2);
      if (valuelen != 2)
        continue;
      byteorder = 1;
      if (!(obj = PyUnicode_DecodeUTF16((char *)codebytes, 2, "strict",
              &byteorder)))
        return NULL;
      *lenp = codelen;
      return obj;
    }
    if (obj->ob_type != &PDFStringType && obj->ob_type != &PyBytes_Type) {
      Py_DECREF(obj);
      continue;
    }
    if (PyBytes_AsStringAndSize((obj->ob_type == &PDFStringType) ?
          ((PDFString *)obj)->raw : obj, (char **)&codebytes,
          &valuelen) < 0) {
      Py_DECREF(obj);
      return NULL;
    }
    Py_DECREF(obj);
    if (valuelen != 2)
      continue;
    offset += (codebytes[0] << 8) | codebytes[1];
    resultbytes[0] = (offset >> 8) & 0xff;
    resultbytes[1] = offset & 0xff;
    byteorder = 1;
    if (!(obj = PyUnicode_DecodeUTF16((char *)resultbytes, 2, "strict",
            &byteorder)))
      return NULL;
    *lenp = codelen;
    return obj;
  }
  Py_INCREF(Py_None);
  return Py_None;
}


static PyObject *cmap_match_bfchar(PyObject *chars, const unsigned char *data,
    Py_ssize_t *lenp) {
  Py_ssize_t charslen;
  Py_ssize_t codelen;
  Py_ssize_t i;
  Py_ssize_t valuelen;
  int byteorder;

  if ((charslen = PySequence_Length(chars)) < 0)
    return NULL;
  for (i = 0; i + 1 < charslen - 1; i += 2) {
    PyObject *obj;
    unsigned char *codebytes;
    if (!(obj = PySequence_GetItem(chars, i)))
      return NULL;
    if (PyBytes_AsStringAndSize((obj->ob_type == &PDFStringType) ?
          ((PDFString *)obj)->raw : obj, (char **)&codebytes, &codelen) < 0) {
      Py_DECREF(obj);
      return NULL;
    }
    Py_DECREF(obj);
    if (codelen > *lenp)
      continue;
    if (codelen == 1) {
      if (data[0] != codebytes[0])
        continue;
    } else if (codelen == 2) {
      if (((data[0] << 8) | data[1]) !=
          ((codebytes[0] << 8) | codebytes[1]))
        continue;
    } else {
      if (((data[0] << 16) | (data[1] << 8) | data[2]) !=
          ((codebytes[0] << 16) | (codebytes[1] << 8) | codebytes[2]))
        continue;
    }
    if (!(obj = PySequence_GetItem(chars, i + 1)))
      return NULL;
    if (obj->ob_type == &NameType) {
      PyObject *obj2 = PyObject_GetItem(glyphlist, obj);
      Py_DECREF(obj);
      if (!obj2) {
        PyErr_Clear();
        continue;
      }
      *lenp = codelen;
      return obj2;
    }
    if (obj->ob_type != &PDFStringType && obj->ob_type != &PyBytes_Type) {
      Py_DECREF(obj);
      continue;
    }
    if (PyBytes_AsStringAndSize((obj->ob_type == &PDFStringType) ?
          ((PDFString *)obj)->raw : obj, (char **)&codebytes,
          &valuelen) < 0) {
      Py_DECREF(obj);
      return NULL;
    }
    Py_DECREF(obj);
    if (valuelen != 2)
      continue;
    byteorder = 1;
    if (!(obj = PyUnicode_DecodeUTF16((char *)codebytes, 2, "strict",
            &byteorder)))
      return NULL;
    *lenp = codelen;
    return obj;
  }
  Py_INCREF(Py_None);
  return Py_None;
}


static PyObject *decode_font_string_cmap(PyObject *cmap, const char *data,
    Py_ssize_t len) {
  PyObject *obj = NULL;
  PyObject *operatorobj = NULL;
  PyObject *cmapcontents = NULL;
  PyObject *out = NULL;
  PyObject *iter = NULL;
  PyObject *item = NULL;
  const char *end = data + len;
  const char *operator;
  Py_ssize_t itemlen;
  Py_ssize_t codelen;

  if (!(obj = streamobject_get_data((StreamObject *)cmap, NULL)))
    goto error;
  if (!(cmapcontents = parse_contents(((StreamObject *)cmap)->pdf, obj)))
    goto error;
  Py_CLEAR(obj);
  if (!(out = PyList_New(0)))
    goto error;
  while (data < end) {
    codelen = (end - data) > 3 ? 3 : (end - data);
    if (!(iter = PyObject_GetIter(cmapcontents)))
      goto error;
    while ((item = PyIter_Next(iter))) {
      if (!(obj = PySequence_GetItem(item, -1)))
        goto error;
      if (obj->ob_type == &OperatorType) {
        if (!(operatorobj = PyUnicode_AsASCIIString(obj)))
          goto error;
        Py_CLEAR(obj);
        operator = PyBytes_AS_STRING(operatorobj);
        if ((itemlen = PySequence_Length(item)) < 0)
          goto error;
        if (!strcmp(operator, "endbfchar")) {
          if (!(obj = cmap_match_bfchar(item, (unsigned char *)data,
                  &codelen)))
            goto error;
          if (obj != Py_None) {
            if (PyList_Append(out, obj) < 0)
              goto error;
            Py_CLEAR(obj);
            Py_CLEAR(item);
            break;
          }
          Py_CLEAR(obj);
        } else if (codelen && !strcmp(operator, "endbfrange")) {
          if (!(obj = cmap_match_bfrange(item, (unsigned char *)data,
                  &codelen)))
            goto error;
          if (obj != Py_None) {
            if (PyList_Append(out, obj) < 0)
              goto error;
            Py_CLEAR(obj);
            Py_CLEAR(item);
            break;
          }
          Py_CLEAR(obj);
        }
        Py_CLEAR(operatorobj);
      } else {
        Py_CLEAR(obj);
      }
      Py_CLEAR(item);
    }
    Py_CLEAR(iter);
    if (PyErr_Occurred())
      goto error;
    data += codelen ? codelen : 1;
  }
  obj = PyObject_CallMethod(string_emptystring, "join", "O", out);
  Py_XDECREF(out);
  Py_XDECREF(cmapcontents);
  return obj;
error:
  Py_XDECREF(out);
  Py_XDECREF(obj);
  Py_XDECREF(operatorobj);
  Py_XDECREF(iter);
  Py_XDECREF(item);
  Py_XDECREF(cmapcontents);
  return NULL;
}


static PyObject *decode_font_string(PyObject *font, const char *data,
    Py_ssize_t len) {
  PyObject *obj;
  PyObject *retval;

  if (PyMapping_HasKeyString(font, "ToUnicode")) {
    if (!(obj = PyMapping_GetItemString(font, "ToUnicode")))
      return NULL;
    if (obj->ob_type == &StreamObjectType) {
      retval = decode_font_string_cmap(obj, data, len);
      Py_DECREF(obj);
      if (retval != Py_None)
        return retval;
    } else {
      Py_DECREF(obj);
    }
  }
  if (PyMapping_HasKeyString(font, "Encoding")) {
    if (!(obj = PyMapping_GetItemString(font, "Encoding")))
      return NULL;
    if (obj->ob_type == &DictionaryType) {
      retval = decode_font_string_encoding(obj, data, len);
      Py_DECREF(obj);
      if (retval != Py_None)
        return retval;
      Py_DECREF(retval);
    } else {
      Py_DECREF(obj);
    }
  }
  Py_INCREF(Py_None);
  return Py_None;
}


static int text_append(PyObject *result, PyObject *args, Py_ssize_t argnum,
    PyObject *font) {
  Py_ssize_t len;
  PyObject *obj = NULL;
  PyObject *decoded = NULL;
  char *data;

  if ((len = PySequence_Length(args)) < 0)
    goto error;
  if (argnum >= len)
    return 1;
  if (!(obj = PySequence_GetItem(args, argnum)))
    goto error;
  if (!font) { /* no font known, if it is a PDFString just append it as is */
    if (obj->ob_type == &PDFStringType && PyList_Append(result, obj) < 0)
      goto error;
    goto ok;
  }
  if (obj->ob_type != &PyBytes_Type && obj->ob_type != &PDFStringType)
    goto ok; /* not a string type at all, ignore it */
  if (PyBytes_AsStringAndSize((obj->ob_type == &PDFStringType) ?
        ((PDFString *)obj)->raw : obj, &data, &len) < 0)
    goto error;
  if (!(decoded = decode_font_string(font, data, len)))
    goto error;
  if (decoded == Py_None) {
    if (obj->ob_type == &PDFStringType)
      if (PyList_Append(result, obj) < 0)
        goto error;
  } else if (PyList_Append(result, decoded) < 0) {
    goto error;
  }
ok:
  Py_XDECREF(decoded);
  Py_XDECREF(obj);
  return 1;
error:
  Py_XDECREF(decoded);
  Py_XDECREF(obj);
  return 0;
}


static PyObject *page_get_text(Page *self, void *closure) {
  PyObject *iter = NULL;
  PyObject *entry = NULL;
  PyObject *operatorobj = NULL;
  PyObject *obj = NULL;
  PyObject *result = NULL;
  PyObject *fonts = NULL;
  PyObject *font = NULL;
  Py_ssize_t len;
  Py_ssize_t i;
  const char *operator;

  if (self->text) {
    Py_INCREF(self->text);
    return self->text;
  }

  if (!(obj = page_get_contents(self, NULL)))
    return NULL;
  if (!(iter = PyObject_GetIter(obj)))
    goto error;
  Py_CLEAR(obj);
  if (PyMapping_HasKeyString((PyObject *)self, "Resources")) {
    if (!(obj = PyMapping_GetItemString((PyObject *)self, "Resources")))
      goto error;
    if (obj->ob_type == &DictionaryType &&
        PyMapping_HasKeyString(obj, "Font")) {
      if (!(fonts = PyMapping_GetItemString(obj, "Font")))
        goto error;
      if (fonts->ob_type != &DictionaryType)
        Py_CLEAR(fonts);
    }
    Py_CLEAR(obj);
  }
  if (!(result = PyList_New(0)))
    goto error;
  while ((entry = PyIter_Next(iter))) {
    if (!(obj = PySequence_GetItem(entry, -1)))
      goto error;
    if (obj->ob_type != &OperatorType) {
      pdf_error(NULL, NULL, "Content stream operator is a %s not an Operator",
          obj->ob_type->tp_name);
      goto error;
    }
    if (!(operatorobj = PyUnicode_AsASCIIString(obj)))
      goto error;
    Py_CLEAR(obj);
    operator = PyBytes_AS_STRING(operatorobj);
    if (fonts && !strcmp(operator, "Tf")) {
      Py_CLEAR(font);
      if (!(obj = PySequence_GetItem(entry, 0)))
        goto error;
      if (obj->ob_type == &NameType) {
        if (!(font = PyObject_GetItem(fonts, obj))) {
          PyErr_Clear();
        } else {
          if (font->ob_type != &DictionaryType)
            Py_CLEAR(font);
        }
      }
      Py_CLEAR(obj);
    } else if (!strcmp(operator, "Tj")) {
      if (!text_append(result, entry, 0, font))
        goto error;
    } else if (!strcmp(operator, "T*")) { /* Td and TD too maybe? */
      if (PyList_Append(result, string_newline) < 0)
        goto error;
    } else if (!strcmp(operator, "'")) {
      if (PyList_Append(result, string_newline) < 0)
        goto error;
      if (!text_append(result, entry, 0, font))
        goto error;
    } else if (!strcmp(operator, "\"")) {
      if (PyList_Append(result, string_newline) < 0)
        goto error;
      if (!text_append(result, entry, 2, font))
        goto error;
    } else if (!strcmp(operator, "TJ")) {
      if (!(obj = PySequence_GetItem(entry, 0)))
        goto error;
      if (obj->ob_type == &ArrayType) {
        if ((len = PySequence_Length(obj)) < 0)
          goto error;
        for (i = 0; i < len; i++)
          if (!text_append(result, obj, i, font))
            goto error;
      }
      Py_CLEAR(obj);
    }
    Py_CLEAR(entry);
    Py_CLEAR(operatorobj);
  }
  Py_CLEAR(iter);
  if (PyErr_Occurred())
    goto error;
  obj = PyUnicode_Join(string_emptystring, result);
  Py_XDECREF(result);
  Py_XDECREF(fonts);
  Py_XDECREF(font);
  Py_INCREF(obj);
  self->text = obj;
  return obj;
error:
  Py_XDECREF(iter);
  Py_XDECREF(entry);
  Py_XDECREF(obj);
  Py_XDECREF(operatorobj);
  Py_XDECREF(result);
  Py_XDECREF(fonts);
  Py_XDECREF(font);
  return NULL;
}


static PyGetSetDef page_getset[] = {
  { "contents", (getter)page_get_contents, NULL,
    "The sequence of operations that make up the page contents.", NULL },
  { "text", (getter)page_get_text, NULL,
    "An approximation of the textual content of the page.", NULL },
  { NULL }
};


static int page_clear(Page *self) {
  Py_CLEAR(self->contents);
  Py_CLEAR(self->text);
  return PyDict_Type.tp_clear((PyObject *)self);
}


static int page_traverse(Page *self, visitproc visit, void *arg) {
  Py_VISIT(self->contents);
  Py_VISIT(self->text);
  return PyDict_Type.tp_traverse((PyObject *)self, visit, arg);
}


static void page_dealloc(Page *self) {
  Py_CLEAR(self->contents);
  Py_CLEAR(self->text);
  PyDict_Type.tp_dealloc((PyObject *)self);
}


static PyObject *page_new(PyTypeObject *type, PyObject *args,
    PyObject *kwargs) {
  Page *self;

  if ((self = (Page *)DictionaryType.tp_new(type, args, kwargs))) {
    self->contents = NULL;
    self->text = NULL;
  }
  return (PyObject *)self;
}


static PyTypeObject PageType = {
  PyVarObject_HEAD_INIT(NULL, 0)
  "pycpdf.Page",              /*tp_name*/
  sizeof(Page),               /*tp_basicsize*/
  0,                          /*tp_itemsize*/
  (destructor)page_dealloc,   /*tp_dealloc*/
  0,                          /*tp_print*/
  0,                          /*tp_getattr*/
  0,                          /*tp_setattr*/
  0,                          /*tp_compare*/
  0,                          /*tp_repr*/
  0,                          /*tp_as_number*/
  0,                          /*tp_as_sequence*/
  0,                          /*tp_as_mapping*/
  0,                          /*tp_hash*/
  0,                          /*tp_call*/
  0,                          /*tp_str*/
  0,                          /*tp_getattro*/
  0,                          /*tp_setattro*/
  0,                          /*tp_as_buffer*/
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_GC, /*tp_flags*/
  "PDF Page objects",         /*tp_doc*/
  (traverseproc)page_traverse, /*tp_traverse*/
  (inquiry)page_clear,        /*tp_clear*/
  0,                          /*tp_richcompare*/
  0,                          /*tp_weaklistoffset*/
  0,                          /*tp_iter*/
  0,                          /*tp_iternext*/
  0,                          /*tp_methods*/
  0,                          /*tp_members*/
  page_getset,                /*tp_getset*/
  0,                          /*tp_base*/
  0,                          /*tp_dict*/
  0,                          /*tp_descr_get*/
  0,                          /*tp_descr_set*/
  0,                          /*tp_dictoffset*/
  0,                          /*tp_init*/
  0,                          /*tp_alloc*/
  page_new,                   /*tp_new*/
  0,                          /*tp_free*/
  0,                          /*tp_is_gc*/
};


typedef struct {
  PyLongObject intobj;
} Offset;


static PyTypeObject OffsetType = {
  PyVarObject_HEAD_INIT(NULL, 0)
  "pycpdf.Offset",            /*tp_name*/
  sizeof(Offset),             /*tp_basicsize*/
  0,                          /*tp_itemsize*/
  0,                          /*tp_dealloc*/
  0,                          /*tp_print*/
  0,                          /*tp_getattr*/
  0,                          /*tp_setattr*/
  0,                          /*tp_compare*/
  0,                          /*tp_repr*/
  0,                          /*tp_as_number*/
  0,                          /*tp_as_sequence*/
  0,                          /*tp_as_mapping*/
  0,                          /*tp_hash*/
  0,                          /*tp_call*/
  0,                          /*tp_str*/
  0,                          /*tp_getattro*/
  0,                          /*tp_setattro*/
  0,                          /*tp_as_buffer*/
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
  "Offset positions in the PDF file", /*tp_doc*/
  0,                          /*tp_traverse*/
  0,                          /*tp_clear*/
  0,                          /*tp_richcompare*/
  0,                          /*tp_weaklistoffset*/
  0,                          /*tp_iter*/
  0,                          /*tp_iternext*/
  0,                          /*tp_methods*/
  0,                          /*tp_members*/
  0,                          /*tp_getset*/
  0,                          /*tp_base*/
  0,                          /*tp_dict*/
  0,                          /*tp_descr_get*/
  0,                          /*tp_descr_set*/
  0,                          /*tp_dictoffset*/
  0,                          /*tp_init*/
  0,                          /*tp_alloc*/
  0,                          /*tp_new*/
  0,                          /*tp_free*/
  0,                          /*tp_is_gc*/
};


typedef struct {
  PyLongObject intobj;
  long number;
  long index;
} StreamReference;


static PyObject *streamreference_new(PyTypeObject *type, PyObject *args,
    PyObject *kwargs) {
  StreamReference *self;
  long number;
  long index;

  if (!PyArg_ParseTuple(args, "ll:StreamReference", &number, &index))
    return NULL;
  if (!(self = (StreamReference *)type->tp_alloc(type, 0)))
    return NULL;
  self->number = number;
  self->index = index;
  return (PyObject *)self;
}


static PyObject *streamreference_repr(StreamReference *self) {
  return PyBytes_FromFormat("<StreamReference(%ld, %ld)>", self->number,
      self->index);
}


static PyMemberDef StreamReference_members[] = {
  { "number", T_LONG, offsetof(StreamReference, number), READONLY,
    "Object number of containing stream" },
  { "index", T_LONG, offsetof(StreamReference, index), READONLY,
    "Index of this object within containing stream" },
  { NULL }
};


static PyTypeObject StreamReferenceType = {
  PyVarObject_HEAD_INIT(NULL, 0)
  "pycpdf.StreamReference",   /*tp_name*/
  sizeof(StreamReference),    /*tp_basicsize*/
  0,                          /*tp_itemsize*/
  0,                          /*tp_dealloc*/
  0,                          /*tp_print*/
  0,                          /*tp_getattr*/
  0,                          /*tp_setattr*/
  0,                          /*tp_compare*/
  (reprfunc)streamreference_repr, /*tp_repr*/
  0,                          /*tp_as_number*/
  0,                          /*tp_as_sequence*/
  0,                          /*tp_as_mapping*/
  0,                          /*tp_hash*/
  0,                          /*tp_call*/
  0,                          /*tp_str*/
  0,                          /*tp_getattro*/
  0,                          /*tp_setattro*/
  0,                          /*tp_as_buffer*/
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
  "Reference to an object in an object stream", /*tp_doc*/
  0,                          /*tp_traverse*/
  0,                          /*tp_clear*/
  0,                          /*tp_richcompare*/
  0,                          /*tp_weaklistoffset*/
  0,                          /*tp_iter*/
  0,                          /*tp_iternext*/
  0,                          /*tp_methods*/
  StreamReference_members,    /*tp_members*/
  0,                          /*tp_getset*/
  0,                          /*tp_base*/
  0,                          /*tp_dict*/
  0,                          /*tp_descr_get*/
  0,                          /*tp_descr_set*/
  0,                          /*tp_dictoffset*/
  0,                          /*tp_init*/
  0,                          /*tp_alloc*/
  streamreference_new,        /*tp_new*/
  0,                          /*tp_free*/
  0,                          /*tp_is_gc*/
};


static PyObject *pdf_getxref(PDF *self, long number, long generation) {
  PyObject *dictobj;
  PyObject *numobj;
  PyObject *value;

  if (!(numobj = PyLong_FromLong(generation)))
    return NULL;
  dictobj = PyDict_GetItem(self->xref, numobj);
  Py_DECREF(numobj);
  if (!dictobj) {
    Py_INCREF(Py_None);
    return Py_None;
  }
  if (!(numobj = PyLong_FromLong(number)))
    return NULL;
  value = PyDict_GetItem(dictobj, numobj);
  Py_DECREF(numobj);
  if (!value)
    value = Py_None;
  Py_INCREF(value);
  return value;
}


static int pdf_setxref(PDF *self, long number, long generation,
    PyObject *value, int overwrite) {
  PyObject *dictobj;
  PyObject *numobj;

  if (!(numobj = PyLong_FromLong(generation)))
    return 0;
  if (!(dictobj = PyDict_GetItem(self->xref, numobj))) {
    if (!(dictobj = PyDict_New())) {
      Py_DECREF(numobj);
      return 0;
    }
    if (PyDict_SetItem(self->xref, numobj, dictobj)) {
      Py_DECREF(dictobj);
      Py_DECREF(numobj);
      return 0;
    }
    Py_DECREF(dictobj);
  }
  Py_DECREF(numobj);
  if (!(numobj = PyLong_FromLong(number)))
    return 0;
  if (!overwrite) {
    int check;
    if ((check = PyDict_Contains(dictobj, numobj)) < 0) {
      Py_DECREF(numobj);
      return 0;
    }
    if (check)
      return 1;
  }
  if (PyDict_SetItem(dictobj, numobj, value) < 0) {
    Py_DECREF(numobj);
    return 0;
  }
  Py_DECREF(numobj);
  return 1;
}


struct IndirectObject {
  PyObject_HEAD
  PyObject *weakreflist;
  PDF *pdf;
  PyObject *obj;
  long number;
  long generation;
};


static int indirectobject_clear(IndirectObject *self) {
  Py_CLEAR(self->pdf);
  Py_CLEAR(self->obj);
  return 0;
}


static int indirectobject_traverse(IndirectObject *self, visitproc visit,
    void *arg) {
  Py_VISIT(self->pdf);
  Py_VISIT(self->obj);
  return 0;
}


static void indirectobject_dealloc(IndirectObject *self) {
  if (self->weakreflist)
    PyObject_ClearWeakRefs((PyObject *)self);
  indirectobject_clear(self);
  Py_TYPE(self)->tp_free((PyObject *)self);
}


static PyObject *indirectobject_new(PyTypeObject *type, PyObject *args,
    PyObject *kwargs) {
  IndirectObject *self;
  PDF *pdf;
  long number;
  long generation;

  if (!PyArg_ParseTuple(args, "O!ll:IndirectObject", &PDFType, &pdf, &number,
        &generation))
    return NULL;

  if (!(self = (IndirectObject *)type->tp_alloc(type, 0)))
    return NULL;
  self->weakreflist = NULL;
  Py_INCREF(pdf);
  self->pdf = pdf;
  self->obj = NULL;
  self->number = number;
  self->generation = generation;

  return (PyObject *)self;
}


static PyObject *indirectobject_repr(IndirectObject *self) {
  return PyBytes_FromFormat("<IndirectObject(%ld, %ld)>", self->number,
      self->generation);
}


static PyObject *indirectobject_getObject(IndirectObject *self) {
  PyObject *obj;
  PyObject *key = NULL;
  const char *start;
  long offset;
  char ourbuffer = 0;

  if (self->obj) {
    Py_INCREF(self->obj);
    return self->obj;
  }
  if (!(obj = pdf_getxref(self->pdf, self->number, self->generation)))
    return NULL;
  if (obj->ob_type != &OffsetType && obj->ob_type != &StreamReferenceType) {
    Py_INCREF(obj);
    self->obj = obj;
    return obj;
  }
  if (obj->ob_type == &StreamReferenceType) {
    long number = ((StreamReference *)obj)->number;
    PyObject *value;
    char *data;
    Py_ssize_t len;
    long num;
    long first;
    const char *cp;
    const char *end;

    Py_DECREF(obj);
    key = PyObject_CallFunction((PyObject *)&IndirectObjectType,
        "(Oll)", self->pdf, number, 0);
    if (!key)
      return NULL;
    obj = indirectobject_getObject((IndirectObject *)key);
    Py_DECREF(key);
    if (!obj)
      return NULL;
    if (obj->ob_type != &StreamObjectType) {
      pdf_error(self->pdf, NULL,
          "StreamReference pointed to a %s, not a stream",
          obj->ob_type->tp_name);
      Py_DECREF(obj);
      return NULL;
    }
    if (!get_intval(obj, "First", &first)) {
      Py_DECREF(obj);
      return NULL;
    }
    value = streamobject_get_data((StreamObject *)obj, NULL);
    Py_DECREF(obj);
    if (!value)
      return NULL;
    if (PyBytes_AsStringAndSize((value->ob_type == &PDFStringType) ?
          ((PDFString *)value)->raw : value, &data, &len) < 0) {
      Py_DECREF(value);
      return NULL;
    }
    cp = data;
    end = data + len;
    while (1) {
      cp = skipwhitespace(cp, end);
      if (!read_integer(&cp, end, &num)) {
        Py_DECREF(value);
        return pdf_error(self->pdf, NULL, "StreamReference expected object"
             " %ld in stream %ld but it was not found", self->number, number);
      }
      cp = skipwhitespace(cp, end);
      if (num != self->number) {
        read_integer(&cp, end, &num);
        continue;
      }
      if (!read_integer(&cp, end, &num)) {
        Py_DECREF(value);
        return pdf_error(self->pdf, NULL,
            "Couldn't parse offset for object %ld in stream %ld",
            self->number, number);
      }
      break;
    }
    cp = data + first + num;
    if (cp < data || cp >= end) {
      Py_DECREF(value);
      return pdf_error(self->pdf, NULL,
          "Offset for object %ld in stream %ld points outside stream",
          self->number, number);
    }
    obj = read_object(self->pdf, &cp, end, NULL);
    Py_DECREF(value);
    if (!obj)
      return NULL;
    if (!pdf_setxref(self->pdf, self->number, self->generation, obj, 1)) {
      Py_DECREF(obj);
      return NULL;
    }
    Py_INCREF(obj);
    self->obj = obj;
    return obj;
  }
  offset = PyLong_AS_LONG(obj);
  Py_DECREF(obj);
  if (self->pdf->key) {
    PyObject *md5obj;
    char keybuf[5];
    if (!(md5obj = PyObject_CallObject(md5, NULL)))
      return NULL;
    if (!md5update_str(md5obj, self->pdf->key, 0)) {
      Py_DECREF(md5obj);
      return NULL;
    }
    keybuf[0] = self->number & 255;
    keybuf[1] = (self->number >> 8) & 255;
    keybuf[2] = (self->number >> 16) & 255;
    keybuf[3] = self->generation & 255;
    keybuf[4] = (self->generation >> 8) & 255;
    if (!md5update(md5obj, keybuf, 5)) {
      Py_DECREF(md5obj);
      return NULL;
    }
    obj = PyObject_CallMethod(md5obj, "digest", NULL);
    Py_DECREF(md5obj);
    if (!obj)
      return NULL;
    key = PySequence_GetSlice(obj, 0,
        (PyBytes_GET_SIZE(self->pdf->key) >= 11) ? 16 :
        (PyBytes_GET_SIZE(self->pdf->key) + 5));
    Py_DECREF(obj);
    if (!key)
      return NULL;
  }
  if (!self->pdf->data.buf) {
    if (PyObject_GetBuffer(self->pdf->source, &self->pdf->data, PyBUF_SIMPLE))
      return NULL;
    ourbuffer = 1;
  }
  if (offset < 0 || offset >= self->pdf->data.len) {
    if (ourbuffer) {
      PyBuffer_Release(&self->pdf->data);
      self->pdf->data.buf = NULL;
    }
    return pdf_error(self->pdf, NULL,
        "Offset %ld for indirect object %ld %ld points outside file",
        offset, self->generation, self->number);
  }
  start = ((const char *)self->pdf->data.buf) + offset;
  obj = read_object(self->pdf, &start,
      ((const char *)self->pdf->data.buf) + self->pdf->data.len, key);
  if (ourbuffer) {
    PyBuffer_Release(&self->pdf->data);
    self->pdf->data.buf = NULL;
  }
  Py_XDECREF(key);
  if (!obj)
    return NULL;
  if (obj->ob_type == &IndirectObjectType) {
    Py_DECREF(obj);
    return pdf_error(self->pdf, NULL,
        "Indirect object %ld %ld points to another indirect object",
        self->generation, self->number);
  }
  if (!pdf_setxref(self->pdf, self->number, self->generation, obj, 1)) {
    Py_DECREF(obj);
    return NULL;
  }
  Py_INCREF(obj);
  self->obj = obj;
  return obj;
}


static PyMethodDef IndirectObject_methods[] = {
  { "getObject", (PyCFunction)indirectobject_getObject, METH_NOARGS,
    "Return the object indicated by this reference." },
  { NULL }
};


static PyMemberDef IndirectObject_members[] = {
  { "pdf", T_OBJECT, offsetof(IndirectObject, pdf), READONLY,
    "Associated PDF object" },
  { "number", T_LONG, offsetof(IndirectObject, number), READONLY,
    "Object number" },
  { "generation", T_LONG, offsetof(IndirectObject, generation), READONLY,
    "Generation number" },
  { "object", T_OBJECT, offsetof(IndirectObject, obj), READONLY,
    "Resolved object, or None if not resolved yet." },
  { NULL }
};


static PyTypeObject IndirectObjectType = {
  PyVarObject_HEAD_INIT(NULL, 0)
  "pycpdf.IndirectObject",    /*tp_name*/
  sizeof(IndirectObject),     /*tp_basicsize*/
  0,                          /*tp_itemsize*/
  (destructor)indirectobject_dealloc, /*tp_dealloc*/
  0,                          /*tp_print*/
  0,                          /*tp_getattr*/
  0,                          /*tp_setattr*/
  0,                          /*tp_compare*/
  (reprfunc)indirectobject_repr, /*tp_repr*/
  0,                          /*tp_as_number*/
  0,                          /*tp_as_sequence*/
  0,                          /*tp_as_mapping*/
  0,                          /*tp_hash*/
  0,                          /*tp_call*/
  0,                          /*tp_str*/
  0,                          /*tp_getattro*/
  0,                          /*tp_setattro*/
  0,                          /*tp_as_buffer*/
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_GC, /*tp_flags*/
  "Indirect PDF objects",     /*tp_doc*/
  (traverseproc)indirectobject_traverse, /*tp_traverse*/
  (inquiry)indirectobject_clear, /*tp_clear*/
  0,                          /*tp_richcompare*/
  offsetof(IndirectObject, weakreflist), /*tp_weaklistoffset*/
  0,                          /*tp_iter*/
  0,                          /*tp_iternext*/
  IndirectObject_methods,     /*tp_methods*/
  IndirectObject_members,     /*tp_members*/
  0,                          /*tp_getset*/
  0,                          /*tp_base*/
  0,                          /*tp_dict*/
  0,                          /*tp_descr_get*/
  0,                          /*tp_descr_set*/
  0,                          /*tp_dictoffset*/
  0,                          /*tp_init*/
  0,                          /*tp_alloc*/
  indirectobject_new,         /*tp_new*/
  0,                          /*tp_free*/
  0,                          /*tp_is_gc*/
};


static PyObject *unicode_fromstringandsize(const char *str, Py_ssize_t len) {
  PyObject *strobj;
  PyObject *uniobj;

  if (!(strobj = PyBytes_FromStringAndSize(str, len)))
    return NULL;
  uniobj = PyUnicode_FromEncodedObject(strobj, "utf8", "strict");
  Py_DECREF(strobj);
  return uniobj;
}


static PyObject *decode_string(const char *string, Py_ssize_t len,
    PyObject *decryption_key, char unicode) {
  PyObject *obj = NULL;
  PyObject *raw;
  PyObject *args;
  char *data;
  PDFString *strobj;

  if (decryption_key && !PyBytes_Check(decryption_key)) {
    PyErr_Format(baseerror, "decryption_key is a %s not a string",
        decryption_key->ob_type->tp_name);
    return NULL;
  }
  data = (char *)string;
  if (decryption_key) {
    if (!(data = PyMem_Malloc(len)))
      return PyErr_NoMemory();
    memcpy(data, string, len);
    rc4(PyBytes_AS_STRING(decryption_key),
        PyBytes_GET_SIZE(decryption_key), data, len);
  }
  if (!(raw = PyBytes_FromStringAndSize(data, len))) {
    if (decryption_key)
      PyMem_Free(data);
    return NULL;
  }
  if (!unicode) {
    if (decryption_key)
      PyMem_Free(data);
    return raw;
  }
  if (len == 0 || (len >= 2 && string[0] == '\xfe' && string[1] == '\xff'))
    obj = PyUnicode_DecodeUTF16(data, len, "strict", NULL);
  else
    obj = PyUnicode_DecodeCharmap(data, len, pdfdocencoding, "strict");
  if (decryption_key)
    PyMem_Free(data);
  if (!obj) {
    PyErr_Clear();
    return raw;
  }
  if (!(args = Py_BuildValue("(O)", obj))) {
    Py_DECREF(raw);
    Py_DECREF(obj);
    return NULL;
  }
  strobj = (PDFString *)PyObject_CallObject((PyObject *)&PDFStringType, args);
  Py_DECREF(args);
  Py_DECREF(obj);
  if (!strobj) {
    Py_DECREF(raw);
    return NULL;
  }
  strobj->raw = raw;
  return (PyObject *)strobj;
}

typedef struct {
  const char *name;
  PyObject *(*handler)(PDF *, const char *, Py_ssize_t, PyObject *, long *);
} filterdecoder;


static filterdecoder filterdecoders[];

static PyObject *process_filter(PDF *self, PyObject *value, PyObject *name,
    PyObject *parms, long *usedbytes) {
  PyObject *filter;
  PyObject *ret;
  filterdecoder *filterdecoder;
  char *data;
  Py_ssize_t len;

  if (name->ob_type != &NameType)
    return pdf_error(self, NULL, "Filter name is a %s not a pycpdf.Name",
        name->ob_type->tp_name);
  if (!(filter = PyUnicode_AsASCIIString(name)))
    return pdf_error(self, NULL, "Invalid Filter name");
  if (PyBytes_AsStringAndSize(value, &data, &len) < 0) {
    Py_DECREF(filter);
    return NULL;
  }
  for (filterdecoder = filterdecoders; filterdecoder->name; filterdecoder++) {
    if (!strcmp(filterdecoder->name, PyBytes_AS_STRING(filter))) {
      if (!filterdecoder->handler) {
        PyErr_Format(notsupportederror, "Filter %s is not supported",
            PyBytes_AS_STRING(filter));
        Py_DECREF(filter);
        return NULL;
      }
      Py_DECREF(filter);
      return filterdecoder->handler(self, data, len, parms, usedbytes);
    }
  }
  ret = pdf_error(self, NULL, "Unknown Filter %s", PyBytes_AS_STRING(filter));
  Py_DECREF(filter);
  return ret;
}


typedef struct abbreviation {
  const char *abbreviation;
  const char *expanded;
  struct abbreviation *keyabbreviations;
} abbreviation;


static PyObject *expand_abbreviations(PyObject *key, PyObject *name,
    abbreviation *abbreviations) {
  const char *keystr = NULL;
  const char *namestr;

  if (!(!key || (keystr = PyBytes_AsString(key))) ||
      !(namestr = PyBytes_AsString(name))) {
    PyErr_Clear();
  } else {
    while (abbreviations->abbreviation) {
      if (keystr && !strcmp(keystr, abbreviations->expanded) &&
          abbreviations->keyabbreviations)
        return expand_abbreviations(NULL, name,
            abbreviations->keyabbreviations);
      else if (!keystr && !strcmp(namestr, abbreviations->abbreviation))
        return PyObject_CallFunction((PyObject *)&NameType,
            "s", abbreviations->expanded);
      abbreviations++;
    }
  }
  Py_INCREF(name);
  return name;
}


static abbreviation inline_image_colorspace_abbreviations[] = {
    { "G", "DeviceGray", NULL },
    { "RGB", "DeviceRGB", NULL },
    { "CMYK", "DeviceCMYK", NULL },
    { "I", "Indexed", NULL },
    { NULL }
};


static abbreviation inline_image_filter_abbreviations[] = {
    { "AHx", "ASCIIHexDecode", NULL },
    { "A85", "ASCII85Decode", NULL },
    { "LZW", "LZWDecode", NULL },
    { "Fl", "FlateDecode", NULL },
    { "RL", "RunLengthDecode", NULL },
    { "CCF", "CCITTFaxDecode", NULL },
    { "DCT", "DCTDecode", NULL },
    { NULL }
};


static abbreviation inline_image_abbreviations[] = {
  { "BPC", "BitsPerComponent", NULL },
  { "CS", "ColorSpace", inline_image_colorspace_abbreviations },
  { "D", "Decode", NULL },
  { "DP", "DecodeParms", NULL },
  { "F", "Filter", inline_image_filter_abbreviations },
  { "H", "Height", NULL },
  { "IM", "ImageMask", NULL },
  { "I", "Interpolate", NULL },
  { "W", "Width", NULL },
  { NULL }
};


static PyObject *read_inline_image(PDF *self, const char **start,
    const char *end, PyObject *decryption_key) {
  PyObject *dict;
  PyObject *value;
  PyObject *obj;
  PyObject *filter = NULL;
  PyObject *parms;
  const char *cp;
  long length;

  cp = skipwhitespace(*start, end);
  if (cp + 2 >= end || memcmp(cp, "BI", 2))
    return pdf_error(self, cp, "Inline image missing initial 'BI' operator");

  if (!(dict = PyObject_CallObject((PyObject *)&DictionaryType, NULL)))
    return NULL;
  cp += 2;
  while (1) {
    PyObject *key;
    int result;
    cp = skipwhitespace(cp, end);
    if (cp + 1 >= end) {
      Py_DECREF(dict);
      return pdf_error(self, NULL,
          "End of content when looking for inline image data");
    }
    if (!(key = read_object(self, &cp, end, decryption_key))) {
      Py_DECREF(dict);
      return NULL;
    }
    if (key->ob_type == &OperatorType) {
      PyObject *operatorobj;
      if (!(operatorobj = PyUnicode_AsASCIIString(key))) {
        Py_DECREF(key);
        Py_DECREF(dict);
        return NULL;
      }
      if (strcmp(PyBytes_AS_STRING(operatorobj), "ID")) {
        Py_DECREF(key);
        Py_DECREF(dict);
        Py_DECREF(operatorobj);
        return pdf_error(self, cp,
            "Unexpected operator when reading inline image dictionary");
      }
      Py_DECREF(operatorobj);
      Py_DECREF(key);
      if (cp >= end || !is_pdfwhitespace(*cp)) {
        Py_DECREF(dict);
        return pdf_error(self, cp,
            "Missing whitespace character after inline image ID operator");
      }
      cp++;
      if (PyMapping_SetItemString(dict, "Type", name_inlineimage) < 0) {
        Py_DECREF(dict);
        return NULL;
      }
      if (PyMapping_HasKeyString(dict, "Filter")) {
        if (!(filter = PyMapping_GetItemString(dict, "Filter"))) {
          Py_DECREF(dict);
          return NULL;
        }
        if (filter->ob_type == &ArrayType) {
          if ((length = PySequence_Length(filter)) < 0) {
            Py_DECREF(filter);
            Py_DECREF(dict);
            return NULL;
          }
          if (length >= 1) {
            if (!(obj = PySequence_GetItem(filter, 0))) {
              Py_DECREF(filter);
              Py_DECREF(dict);
              return NULL;
            }
            Py_DECREF(filter);
            filter = obj;
          } else {
            Py_DECREF(filter);
            filter = NULL;
          }
        }
      }
      if (!filter) {
        /* no filter, work out image length from size and depth */
        long colors = 0;
        long bitspercomponent = 8;
        long width;
        long height;
        long imagelen;

        if (PyMapping_HasKeyString(dict, "BitsPerComponent")) {
          if (!get_intval(dict, "BitsPerComponent", &bitspercomponent)) {
            Py_DECREF(dict);
            return NULL;
          }
        }
        if (!get_intval(dict, "Width", &width)) {
          Py_DECREF(dict);
          return NULL;
        }
        if (!get_intval(dict, "Height", &height)) {
          Py_DECREF(dict);
          return NULL;
        }
        if (PyMapping_HasKeyString(dict, "ImageMask")) {
          if (!(obj = PyMapping_GetItemString(dict, "ImageMask"))) {
            Py_DECREF(dict);
            return NULL;
          }
          if (obj != Py_False)
            colors = 1;
          Py_DECREF(obj);
        }
        if (!colors) {
          const char *colorspace;
          if (!(obj = PyMapping_GetItemString(dict, "ColorSpace"))) {
            Py_DECREF(dict);
            return NULL;
          }
          if (!(colorspace = PyBytes_AsString(obj))) {
            Py_DECREF(obj);
            Py_DECREF(dict);
            return NULL;
          }
          if (!strcmp(colorspace, "DeviceRGB"))
            colors = 3;
          else if (!strcmp(colorspace, "DeviceGray"))
            colors = 1;
          else if (!strcmp(colorspace, "DeviceCMYK"))
            colors = 4;
          Py_DECREF(obj);
        }
        if (!colors || width < 0 || height < 0 || bitspercomponent < 0) {
          Py_DECREF(dict);
          PyErr_SetString(notsupportederror,
              "Couldn't determine size of inline image");
          return NULL;
        }
        imagelen = height * ((width * bitspercomponent * colors + 7) >> 3);
        if (!(value = PyBytes_FromStringAndSize(cp, imagelen))) {
          Py_DECREF(dict);
          return NULL;
        }
        cp += imagelen;
      } else { /* image data length can be worked out from filter */
        if (PyMapping_HasKeyString(dict, "DecodeParms")) {
          if (!(parms = PyMapping_GetItemString(dict, "DecodeParms"))) {
            Py_DECREF(filter);
            Py_DECREF(dict);
            return NULL;
          }
        } else {
          parms = Py_None;
          Py_INCREF(Py_None);
        }
        if (!(obj = PyBytes_FromStringAndSize(cp, end - cp))) {
          Py_DECREF(parms);
          Py_DECREF(filter);
          Py_DECREF(dict);
          return NULL;
        }
        value = read_filtered_data(self, obj, filter, parms, &length);
        Py_DECREF(obj);
        Py_DECREF(parms);
        Py_DECREF(filter);
        if (!value) {
          Py_DECREF(dict);
          return NULL;
        }
        Py_DECREF(value);
        if (!(value = PyBytes_FromStringAndSize(cp, length))) {
          Py_DECREF(dict);
          return NULL;
        }
        cp += length;
      }
      if (!(obj = read_object(self, &cp, end, decryption_key))) {
        Py_DECREF(value);
        Py_DECREF(dict);
        return NULL;
      }
      if (obj->ob_type != &OperatorType) {
        pdf_error(self, cp,
            "Object after inline image data is a %s not a pycpdf.Operator",
            obj->ob_type->tp_name);
        Py_DECREF(obj);
        Py_DECREF(value);
        Py_DECREF(dict);
      }
      if (!(operatorobj = PyUnicode_AsASCIIString(obj))) {
        Py_DECREF(obj);
        Py_DECREF(value);
        Py_DECREF(dict);
        return NULL;
      }
      if (strcmp(PyBytes_AS_STRING(operatorobj), "EI")) {
        Py_DECREF(operatorobj);
        Py_DECREF(obj);
        Py_DECREF(value);
        Py_DECREF(dict);
        return pdf_error(self, cp, "Missing EI operator after inline image");
      }
      Py_DECREF(operatorobj);
      Py_DECREF(obj);
      obj = PyObject_CallFunction((PyObject *)&StreamObjectType, "OOO", self,
          dict, value);
      Py_DECREF(dict);
      Py_DECREF(value);
      *start = cp;
      return obj;
    } else if (key->ob_type != &NameType) {
      pdf_error(self, cp,
          "Inline image dictionary key is a %s not a pycpdf.Name",
          key->ob_type->tp_name);
      Py_DECREF(key);
      Py_DECREF(dict);
      return NULL;
    }
    if (!(obj = expand_abbreviations(NULL, key,
            inline_image_abbreviations))) {
      Py_DECREF(key);
      Py_DECREF(dict);
      return NULL;
    }
    Py_DECREF(key);
    key = obj;
    if (!(value = read_object(self, &cp, end, decryption_key))) {
      Py_DECREF(key);
      Py_DECREF(dict);
      return NULL;
    }
    if (!(obj = expand_abbreviations(key, value,
            inline_image_abbreviations))) {
      Py_DECREF(value);
      Py_DECREF(key);
      Py_DECREF(dict);
      return NULL;
    }
    Py_DECREF(value);
    value = obj;
    result = value != Py_None ? PyDict_SetItem(dict, key, value) : 0;
    Py_DECREF(key);
    Py_DECREF(value);
    if (result) {
      Py_DECREF(dict);
      return NULL;
    }
  }
}


static PyObject *read_object(PDF *self, const char **start, const char *end,
    PyObject *decryption_key) {
  const char *cp = *start;
  PyObject *obj;

  cp = skipwhitespace(cp, end);
  if (cp >= end)
    return pdf_error(self, NULL, "EOF while looking for token");

  if (cp + 4 <= end && !memcmp(cp, "null", 4)) {
    *start = cp + 4;
    Py_INCREF(Py_None);
    return Py_None;
  }
  if (cp + 4 <= end && !memcmp(cp, "true", 4)) {
    *start = cp + 4;
    Py_INCREF(Py_True);
    return Py_True;
  }
  if (cp + 5 <= end && !memcmp(cp, "false", 5)) {
    *start = cp + 5;
    Py_INCREF(Py_False);
    return Py_False;
  }
  if (cp + 1 < end && *cp == '/') { /* name */
    char *decoded;
    Py_ssize_t len;

    cp++;
    len = 1;
    while (cp + len < end && !is_pdfwhitespace(cp[len]) &&
        !is_pdfdelimiter(cp[len]))
      len++;
    if (!(decoded = PyMem_Malloc(len)))
      return PyErr_NoMemory();
    len = 0;
    while (cp < end && !is_pdfwhitespace(*cp) && !is_pdfdelimiter(*cp)) {
      if (*cp != '#') {
        decoded[len++] = *(cp++);
        continue;
      }
      if (cp + 2 >= end || decodexdigit(cp[1]) == 16 ||
          decodexdigit(cp[2]) == 16) {
        PyMem_Free(decoded);
        return pdf_error(self, cp, "Bad character or EOF in Name '#' escape");
      }
      decoded[len++] = (decodexdigit(cp[1]) << 4) + decodexdigit(cp[2]);
      cp += 2;
    }
    obj = PyObject_CallFunction((PyObject *)&NameType, "(s#)",
        decoded, len);
    PyMem_Free(decoded);
    *start = cp;
    return obj;
  }
  if (*cp == '+' || *cp == '-' || *cp == '.' || (*cp >= '0' && *cp <= '9')) {
    /* integer or real or indirect object or indirect object reference */
    long intval;
    double floatval;
    long num, gen;

    if (read_twointegers(&cp, end, &num, &gen, "obj")) { /* indirect object */
      if (!(obj = read_object(self, &cp, end, decryption_key)))
        return NULL;
      cp = skipwhitespace(cp, end);
      if (cp + 6 >= end || memcmp(cp, "endobj", 6)) {
        Py_DECREF(obj);
        return pdf_error(self, cp,
            "Missing endobj after indirect definition");
      }
      *start = cp + 6;
      if (!pdf_setxref(self, num, gen, obj, 1)) {
        Py_DECREF(obj);
        return NULL;
      }
      return obj;
    }
    if (read_twointegers(&cp, end, &num, &gen, "R")) {
      /* indirect reference */
      if (!(obj = PyObject_CallFunction((PyObject *)&IndirectObjectType,
          "Oll", self, num, gen)))
        return NULL;
      *start = cp;
      return obj;
    }
    if (read_integer(&cp, end, &intval)) { /* integer */
      if (!(obj = PyLong_FromLong(intval)))
        return NULL;
      *start = cp;
      return obj;
    }
    if (read_float(&cp, end, &floatval)) { /* float */
      if (!(obj = PyFloat_FromDouble(floatval)))
        return NULL;
      *start = cp;
      return obj;
    }
  }
  if (*cp == '(') { /* literal string */
    int brackets;
    const char *cp2;
    char *decoded;
    Py_ssize_t len;

    cp++;
    len = 0;
    brackets = 1;
    cp2 = cp;
    while (cp2 < end && brackets > 0) {
      len++;
      if (*cp2 == '(') {
        brackets++;
      } else if (*cp2 == ')') {
        brackets--;
      } else if (*cp2 == '\\') {
        cp2++;
      }
      cp2++;
    }
    if (!(decoded = PyMem_Malloc(len)))
      return PyErr_NoMemory();
    len = 0;
    brackets = 1;
    while (cp < end && brackets > 0) {
      if (*cp == '(') {
        brackets++;
        decoded[len++] = '(';
        cp++;
        continue;
      } else if (*cp == ')') {
        brackets--;
        if (brackets)
          decoded[len++] = ')';
        cp++;
        continue;
      }
      if (*cp == '\r') {
        decoded[len++] = 0x0a;
        cp++;
        if (cp < end && *cp == '\n')
          cp++;
        continue;
      }
      if (*cp != '\\' || cp + 1 >= end) {
        decoded[len++] = *(cp++);
        continue;
      }
      cp++;
      switch (*cp) {
        case 'n':
          decoded[len++] = 0x0a;
          cp++;
          continue;
        case 'r':
          decoded[len++] = 0x0d;
          cp++;
          continue;
        case 't':
          decoded[len++] = 0x09;
          cp++;
          continue;
        case 'b':
          decoded[len++] = 0x08;
          cp++;
          continue;
        case 'f':
          decoded[len++] = 0x0c;
          cp++;
          continue;
        case '\r':
          cp++;
          if (cp < end && *cp == '\n')
            cp++;
          continue;
        case '\n':
          cp++;
          continue;
        case '0': case '1': case '2': case '3':
        case '4': case '5': case '6': case '7':
          decoded[len] = *(cp++) - '0';
          if (cp < end && *cp >= '0' && *cp <= '7')
            decoded[len] = (decoded[len] << 3) | (*(cp++) - '0');
          if (cp < end && *cp >= '0' && *cp <= '7')
            decoded[len] = (decoded[len] << 3) | (*(cp++) - '0');
          len++;
          continue;
        default:
          decoded[len++] = *(cp++);
          continue;
      }
    }
    obj = decode_string(decoded, len, decryption_key, 1);
    PyMem_Free(decoded);
    *start = cp;
    return obj;
  }
  if (cp + 1 < end && *cp == '<' && cp[1] != '<') { /* hexadecimal string */
    const char *cp2;
    char *decoded;
    Py_ssize_t len;

    cp++;
    for (cp2 = cp; cp2 < end && *cp2 != '>'; cp2++)
      ;
    if (!(decoded = PyMem_Malloc(1 + (cp2 - cp) / 2)))
      return PyErr_NoMemory();
    len = 0;
    while (1) {
      char c;
      cp = skipwhitespace(cp, end);
      if (cp >= end) {
        PyMem_Free(decoded);
        return pdf_error(self, cp, "EOF when reading hexadecimal string");
      }
      if (*cp == '>') {
        cp++;
        break;
      }
      if ((decoded[len] = decodexdigit(*cp)) == 16) {
        PyMem_Free(decoded);
        return pdf_error(self, cp, "Bad character in hexadecimal string");
      }
      cp = skipwhitespace(cp + 1, end);
      if (cp >= end) {
        PyMem_Free(decoded);
        return pdf_error(self, cp, "EOF when reading hexadecimal string");
      }
      if (*cp == '>') {
        decoded[len] = decoded[len] << 4;
        len++;
        cp++;
        break;
      }
      if ((c = decodexdigit(*cp)) == 16) {
        PyMem_Free(decoded);
        return pdf_error(self, cp, "Bad character in hexadecimal string");
      }
      decoded[len] = (decoded[len] << 4) | c;
      len++;
      cp++;
    }
    obj = decode_string(decoded, len, decryption_key, 1);
    PyMem_Free(decoded);
    *start = cp;
    return obj;
  }
  if (cp + 1 < end && *cp == '<' && cp[1] == '<') { /* dictionary or stream */
    PyObject *dict;
    PyObject *value;

    if (!(dict = PyObject_CallObject((PyObject *)&DictionaryType, NULL)))
      return NULL;
    cp += 2;
    while (1) {
      PyObject *key;
      int result;
      cp = skipwhitespace(cp, end);
      if (cp + 1 >= end) {
        Py_DECREF(dict);
        return pdf_error(self, NULL, "EOF when looking for dictionary key");
      }
      if (*cp == '>' && cp[1] == '>') {
        long length;

        *start = cp + 2;
        cp = skipwhitespace(cp + 2, end);
        if (cp + 6 >= end || memcmp(cp, "stream", 6)) {
          /* normal dictionary */
          if (PyMapping_HasKeyString(dict, "Type")) {
            int cmp;
            if (!(obj = PyMapping_GetItemString(dict, "Type"))) {
              Py_DECREF(dict);
              return 0;
            }
            cmp = PyObject_RichCompareBool(obj, string_page, Py_EQ);
            Py_DECREF(obj);
            if (cmp < 0)
              return 0;
            if (cmp) { /* it's a Page dictionary */
              obj = PyObject_CallFunctionObjArgs((PyObject *)&PageType,
                  dict, NULL);
              Py_DECREF(dict);
              return obj;
            }
          }
          return dict;
        }
        /* stream */
        if (!(obj = PyMapping_GetItemString(dict, "Length"))) {
          Py_DECREF(dict);
          return NULL;
        }
        if (!PyLong_Check(obj)) {
          pdf_error(self, cp, "Stream Length is a %s not an integer",
              obj->ob_type->tp_name);
          Py_DECREF(obj);
          Py_DECREF(dict);
          return NULL;
        }
        length = PyLong_AS_LONG(obj);
        Py_DECREF(obj);
        if (length < 0) {
          Py_DECREF(dict);
          return pdf_error(self, cp, "Stream Length is negative");
        }
        for ( cp += 6; cp < end && *cp != '\n' && is_pdfwhitespace(*cp); cp++)
          ;
        if (cp >= end || *cp != '\n') {
          Py_DECREF(dict);
          return pdf_error(self, cp, "Missing newline after 'stream'");
        }
        cp++;
        if (cp + length > end) {
          Py_DECREF(dict);
          return pdf_error(self, cp,
              "Stream Length of %ld extends beyond end of file", length);
        }
        if (!(value = decode_string(cp, length, decryption_key, 0))) {
          Py_DECREF(dict);
          return NULL;
        }
        cp = skipwhitespace(cp + length, end);
        if (cp + 9 >= end || memcmp(cp, "endstream", 9)) {
          Py_DECREF(value);
          Py_DECREF(dict);
          return pdf_error(self, cp, "Missing 'endstream' after stream");
        }
        obj = PyObject_CallFunction((PyObject *)&StreamObjectType, "OOO",
            self, dict, value);
        Py_DECREF(value);
        Py_DECREF(dict);
        *start = cp + 9;
        return obj;
      }
      if (*cp != '/') {
        Py_DECREF(dict);
        return pdf_error(self, cp,
            "Unexpected token while looking for dictionary key");
      }
      if (!(key = read_object(self, &cp, end, decryption_key))) {
        Py_DECREF(dict);
        return NULL;
      }
      if (!(value = read_object(self, &cp, end, decryption_key))) {
        Py_DECREF(dict);
        Py_DECREF(key);
        return NULL;
      }
      result = value != Py_None ? PyDict_SetItem(dict, key, value) : 0;
      Py_DECREF(key);
      Py_DECREF(value);
      if (result) {
        Py_DECREF(dict);
        return NULL;
      }
    }
  }
  if (*cp == '[') { /* array */
    PyObject *list;

    if (!(list = PyObject_CallObject((PyObject *)&ArrayType, NULL)))
      return NULL;

    cp++;
    while (1) {
      PyObject *value;
      int result;
      cp = skipwhitespace(cp, end);
      if (cp >= end) {
        Py_DECREF(list);
        return pdf_error(self, NULL, "EOF while looking for array element");
      }
      if (*cp == ']') {
        *start = cp + 1;
        return list;
      }
      if (!(value = read_object(self, &cp, end, decryption_key))) {
        Py_DECREF(list);
        return NULL;
      }
      result = PyList_Append(list, value);
      Py_DECREF(value);
      if (result) {
        Py_DECREF(list);
        return NULL;
      }
    }
  }
  if (cp < end && *cp >= '!' && *cp <= '~' && !is_pdfdelimiter(*cp)) {
    /* operator */
    const char *cp2;

    for (cp2 = cp; cp2 < end && *cp2 >= '!' && *cp2 <= '~' &&
        !is_pdfdelimiter(*cp2); cp2++)
      ;
    if (cp2 - cp == 2 && !memcmp(cp, "BI", 2)) { /* inline image */
      *start = cp;
      return read_inline_image(self, start, end, decryption_key);
    }
    *start = cp2;
    return PyObject_CallFunction((PyObject *)&OperatorType,
        "(s#)", cp, cp2 - cp);
  }
  return pdf_error(self, cp, "Unknown token");
}


static long read_xrefstream_value(const char *start, long nbytes, long def) {
  int i;
  long value;

  if (nbytes == 0)
    return def;
  value = 0;
  for (i = 0; i < nbytes; i++)
    value = (value << 8) | (unsigned char)start[i];
  return value;
}


static int initial_parse(PDF *self, const char *start, const char *end) {
  const char *cp;
  const char *eol;
  int cmp;
  long startxref, num, size, offset, generation;
  PyObject *obj;
  PyObject *obj2;
  PyObject *key;
  PyObject *value;
  PyObject *offsetobj;
  Py_ssize_t pos;

  /* check file header */

  if (start + 9 > end || memcmp(start, "%PDF-", 5) || start[5] < '0'
      || start[5] > '9' || start[6] != '.' || start[7] < '0'
      || start[7] > '9') {
    pdf_error(self, NULL, "Missing PDF file header");
    return 0;
  }
  for (cp = start + 7; cp < end && *cp >= '0' && *cp <= '9'; cp++)
    ;
  if (!(self->version = unicode_fromstringandsize(
          start + 5, cp - (start + 5))))
    return 0;

  /* check for linearization dictionary */

  while (cp < end && *cp != '\r' && *cp != '\n')
    cp++;
  cp = skipwhitespace(cp, end);
  if (read_twointegers(&cp, end, &num, &generation, "obj")) {
    cp = skipwhitespace(cp, end);
    if (cp + 2 < end && *cp == '<' && cp[1] == '<') {
      if (!(obj = read_object(self, &cp, end, NULL)))
        PyErr_Clear();
      if (obj && obj->ob_type == &DictionaryType &&
          PyMapping_HasKeyString(obj, "Linearized")) {
        Py_INCREF(obj);
        self->linearized = obj;
      }
      Py_XDECREF(obj);
    }
  }

  /* find file trailer */

  eol = end;
  cp = end - 1;
  while (cp >= start && *cp != '\r' && *cp != '\n')
    cp--;
  cp++;
  do {
    if (eol - cp == 5 && !memcmp(cp, "%%EOF", 5))
      break;
    if (!(cp = previous_line(cp, &eol, start)) || cp < end - 1024) {
      pdf_error(self, NULL, "Could not find %%%%EOF marker");
      return 0;
    }
  } while (1);

  if (!(cp = previous_line(cp, &eol, start))) {
    pdf_error(self, cp, "Missing 'startxref' marker");
    return 0;
  }

  if (!read_integer(&cp, eol, &startxref)) {
    if (eol - cp > 10 && !memcmp(cp, "startxref", 9)) {
      for (cp += 9; cp < eol && is_pdfwhitespace(*cp); cp++)
        ;
      if (!read_integer(&cp, eol, &startxref)) {
        pdf_error(self, cp, "Unable to parse startxref value");
        return 0;
      }
    } else {
      pdf_error(self, cp, "Unable to parse startxref value");
      return 0;
    }
  } else {
    if (!(cp = previous_line(cp, &eol, start)) || eol - cp < 9 ||
        memcmp(cp, "startxref", 9)) {
      pdf_error(self, cp, "Missing 'startxref' marker");
      return 0;
    }
  }

  while (1) { /* loop through xref tables */
    if (startxref == -1)
      break;
    if (startxref < 0 || start + startxref >= end) {
      pdf_error(self, NULL, "startxref value of %ld points outside file",
          startxref);
      return 0;
    }
    cp = skipwhitespace(start + startxref, end);
    if (cp >= end)
      break;
    if (cp + 4 < end && !memcmp(cp, "xref", 4)) {
      cp = skipwhitespace(cp + 4, end);
      while (read_twointegers(&cp, end, &num, &size, NULL)) {
        for (; size > 0; size--) {
          if (read_twointegers(&cp, end, &offset, &generation, "f")) {
            num++;
            continue;
          }
          if (!read_twointegers(&cp, end, &offset, &generation, "n")) {
            pdf_error(self, cp, "Couldn't parse xref entry");
            return 0;
          }
          if (!(offsetobj = PyObject_CallFunction((PyObject *)&OffsetType,
                  "l", offset)))
            return 0;
          if (!pdf_setxref(self, num, generation, offsetobj, 0)) {
            Py_DECREF(offsetobj);
            return 0;
          }
          Py_DECREF(offsetobj);
          num++;
        }
      }
      cp = skipwhitespace(cp, end);
      startxref = -1;
      if (cp + 7 >= end || memcmp(cp, "trailer", 7))
        continue;
      cp = skipwhitespace(cp + 7, end);
      if (cp + 2 >= end || *cp != '<' || cp[1] != '<') {
        pdf_error(self, cp, "Unknown trailer format");
        return 0;
      }
      if (!(obj = read_object(self, &cp, end, NULL)))
        return 0;
    } else if (read_twointegers(&cp, end, &num, &generation, "obj")) {
      /* cross-reference stream */
      PyObject *index;
      long fields[3];
      long type;
      long rowlen;
      int i;
      int indexpos, indexlen;
      char *data;
      const char *cp2;
      Py_ssize_t len;

      if (!(obj = read_object(self, &cp, end, NULL)))
        return 0;
      if (obj->ob_type != &StreamObjectType) {
        pdf_error(self, start + startxref,
            "Found unexpected %s object in xref section",
            obj->ob_type->tp_name);
        Py_DECREF(obj);
        return 0;
      }
      if (!(value = PyMapping_GetItemString(obj, "W"))) {
        Py_DECREF(obj);
        return 0;
      }
      if (value->ob_type != &ArrayType) {
        pdf_error(self, start + startxref,
            "XRef StreamObject W value is a %s not an array",
            value->ob_type->tp_name);
        Py_DECREF(value);
        Py_DECREF(obj);
        return 0;
      }
      if ((i = PySequence_Length(value)) != 3) {
        if (i >= 0)
          pdf_error(self, start + startxref,
              "XRef StreamObject W value has %d entries not 3", i);
        Py_DECREF(value);
        Py_DECREF(obj);
        return 0;
      }
      rowlen = 0;
      for (i = 0; i < 3; i++) {
        if (!(obj2 = PySequence_GetItem(value, i))) {
          Py_DECREF(value);
          Py_DECREF(obj);
          return 0;
        }
        if (!PyLong_Check(obj2)) {
          pdf_error(self, start + startxref,
              "XRef StreamObject W value is a %s not an integer",
              obj2->ob_type->tp_name);
          Py_DECREF(obj2);
          Py_DECREF(value);
          Py_DECREF(obj);
          return 0;
        }
        fields[i] = PyLong_AS_LONG(obj2);
        Py_DECREF(obj2);
        if (fields[i] < 0 || fields[i] > 4 || (i == 1 && fields[i] < 1)) {
          pdf_error(self, start + startxref,
              "Invalid XRef StreamObject field width %ld", fields[i]);
          Py_DECREF(value);
          Py_DECREF(obj);
          return 0;
        }
        rowlen += fields[i];
      }
      Py_DECREF(value);
      if (!(obj2 = streamobject_get_data((StreamObject *)obj, NULL))) {
        Py_DECREF(obj);
        return 0;
      }
      value = (obj2->ob_type == &PDFStringType) ?
        ((PDFString *)obj2)->raw : obj2;
      if (!PyBytes_Check(value)) {
        pdf_error(self, start + startxref,
            "XRef StreamObject content is a %s not a string",
            value->ob_type->tp_name);
        Py_DECREF(obj2);
        Py_DECREF(obj);
        return 0;
      }
      if (PyBytes_AsStringAndSize(value, &data, &len) < 0) {
        Py_DECREF(obj2);
        Py_DECREF(obj);
        return 0;
      }
      Py_DECREF(obj2);
      num = 0;
      size = LONG_MAX;
      indexpos = 0;
      indexlen = 2;
      index = NULL;
      if (PyMapping_HasKeyString(obj, "Index")) {
        if (!(index = PyMapping_GetItemString(obj, "Index"))) {
          Py_DECREF(obj);
          return 0;
        }
        if (index->ob_type != &ArrayType) {
          pdf_error(self, cp,
              "XRef StreamObject Index value is a %s not an array",
              value->ob_type->tp_name);
          Py_DECREF(index);
          Py_DECREF(obj);
          return 0;
        }
        indexlen = PySequence_Length(index);
        if (indexlen < 0 || indexlen & 1) {
          if (indexlen >= 0)
            pdf_error(self, cp, "XRef StreamObject Index value has %d entries"
                " which is not an even number", indexlen);
          Py_DECREF(index);
          Py_DECREF(obj);
          return 0;
        }
      }
      cp2 = data;
      for (indexpos = 0; indexpos < indexlen; indexpos += 2) {
        if (index) {
          if (!(obj2 = PySequence_GetItem(index, indexpos))) {
            Py_DECREF(index);
            Py_DECREF(obj);
            return 0;
          }
          if (!PyLong_Check(obj2)) {
            pdf_error(self, cp,
                "XRef StreamObject Index value is a %s not an integer",
                obj2->ob_type->tp_name);
            Py_DECREF(obj2);
            Py_DECREF(index);
            Py_DECREF(obj);
            return 0;
          }
          num = PyLong_AS_LONG(obj2);
          Py_DECREF(obj2);
          if (!(obj2 = PySequence_GetItem(index, indexpos + 1))) {
            Py_DECREF(index);
            Py_DECREF(obj);
            return 0;
          }
          if (!PyLong_Check(obj2)) {
            pdf_error(self, cp,
                "XRef StreamObject Index value is a %s not an integer",
                obj2->ob_type->tp_name);
            Py_DECREF(obj2);
            Py_DECREF(index);
            Py_DECREF(obj);
            return 0;
          }
          size = PyLong_AS_LONG(obj2);
          Py_DECREF(obj2);
        }
        while (size > 0 && cp2 + rowlen <= data + len) {
          type = read_xrefstream_value(cp2, fields[0], 1);
          switch (type) {
            case 0: /* free object */
              break;
            case 1: /* uncompressed object */
              offset = read_xrefstream_value(cp2 + fields[0], fields[1], 0);
              generation = read_xrefstream_value(cp2 + fields[0] + fields[1],
                  fields[2], 0);
              offsetobj = PyObject_CallFunction((PyObject *)&OffsetType,
                  "(l)", offset);
              if (!offsetobj) {
                Py_DECREF(index);
                Py_DECREF(obj);
                return 0;
              }
              if (!pdf_setxref(self, num, generation, offsetobj, 0)) {
                Py_DECREF(offsetobj);
                Py_DECREF(index);
                Py_DECREF(obj);
                return 0;
              }
              Py_DECREF(offsetobj);
              break;
            case 2: /* compressed object */
              offset = read_xrefstream_value(cp2 + fields[0], fields[1], 0);
              generation = read_xrefstream_value(cp2 + fields[0] + fields[1],
                  fields[2], 0);
              if (!(offsetobj = PyObject_CallFunction(
                      (PyObject *)&StreamReferenceType,
                      "(ll)", offset, generation))) {
                Py_DECREF(index);
                Py_DECREF(obj);
                return 0;
              }
              if (!pdf_setxref(self, num, 0, offsetobj, 0)) {
                Py_DECREF(offsetobj);
                Py_DECREF(index);
                Py_DECREF(obj);
                return 0;
              }
              Py_DECREF(offsetobj);
              break;
            default:
              pdf_error(self, cp,
                  "XRef StreamObject unknown entry type %ld for object %ld",
                  type, num);
              Py_DECREF(index);
              Py_DECREF(obj);
              return 0;
          }
          cp2 += rowlen;
          num++;
          size--;
        }
      }
      Py_XDECREF(index);
      startxref = -1;
    } else {
      pdf_error(self, cp, "Unknown data in xref table");
      return 0;
    }
    pos = 0;
    while (PyDict_Next(obj, &pos, &key, &value)) {
      if ((cmp = PyObject_RichCompareBool(key, string_prev, Py_EQ)) < 0) {
        Py_DECREF(obj);
        return 0;
      }
      if (cmp) {
        if (!PyLong_Check(value)) {
          pdf_error(self, cp, "Trailer Prev value is a %s not an integer",
              value->ob_type->tp_name);
          Py_DECREF(obj);
          return 0;
        }
        startxref = PyLong_AS_LONG(value);
      }
      if (!PyDict_GetItem(self->trailer, key)) {
        if (PyDict_SetItem(self->trailer, key, value)) {
          Py_DECREF(obj);
          return 0;
        }
      }
    }
    Py_DECREF(obj);
  } /* loop through xref tables */

  return 1;
}


static int pdf_traverse(PDF *self, visitproc visit, void *arg) {
  Py_VISIT(self->source);
  Py_VISIT(self->version);
  Py_VISIT(self->xref);
  Py_VISIT(self->trailer);
  Py_VISIT(self->catalog);
  Py_VISIT(self->info);
  Py_VISIT(self->linearized);
  Py_VISIT(self->pages);
  return 0;
}


static int pdf_clear(PDF *self) {
  Py_CLEAR(self->source);
  Py_CLEAR(self->version);
  Py_CLEAR(self->xref);
  Py_CLEAR(self->trailer);
  Py_CLEAR(self->key);
  Py_CLEAR(self->catalog);
  Py_CLEAR(self->linearized);
  Py_CLEAR(self->info);
  Py_CLEAR(self->pages);
  return 0;
}


static void pdf_dealloc(PDF *self) {
  if (self->weakreflist)
    PyObject_ClearWeakRefs((PyObject *)self);
  pdf_clear(self);
  Py_TYPE(self)->tp_free((PyObject *)self);
}


static char *decrypt_padding =
  "\x28\xBF\x4E\x5E\x4E\x75\x8A\x41\x64\x00\x4E\x56\xFF\xFA\x01\x08"
  "\x2E\x2E\x00\xB6\xD0\x68\x3E\x80\x2F\x0C\xA9\xFE\x64\x53\x69\x7A";


static PyObject *check_password(PDF *self, PyObject *encrypt,
    PyObject *password) {
  PyObject *md5obj;
  PyObject *obj;
  PyObject *obj2;
  PyObject *idobj;
  long keylen;
  long rev;
  long p;
  int len;
  int ulen;
  char passbuf[32];
  char ok;
  const char *keystr;

  if (!get_intval(encrypt, "R", &rev))
    return NULL;
  if (rev == 2) {
    keylen = 40;
  } else {
    if (!get_intval(encrypt, "Length", &keylen))
      return NULL;
    if (keylen < 0 || keylen > 256 || (keylen & 7))
      return pdf_error(self, NULL, "Invalid encryption key Length of %ld",
          keylen);
  }
  keylen >>= 3;
  len = 0;
  if (password) {
    PyObject *passobj;
    char *str;
    Py_ssize_t str_len;

    if (!(passobj = PyUnicode_AsUTF8String(password)))
      return NULL;
    if (PyBytes_AsStringAndSize(passobj, &str, &str_len) < 0) {
      Py_DECREF(passobj);
      return NULL;
    }
    for (; len < 32 && len < str_len; len++)
      passbuf[len] = str[len];
    Py_DECREF(passobj);
  }
  for (; len < 32; len++)
    passbuf[len] = decrypt_padding[len];
  if (!(md5obj = PyObject_CallObject(md5, NULL)))
    return NULL;
  if (!md5update(md5obj, passbuf, 32)) {
    Py_DECREF(md5obj);
    return NULL;
  }
  if (!(obj = PyMapping_GetItemString(encrypt, "O"))) {
    Py_DECREF(md5obj);
    return NULL;
  }
  if (!md5update_str(md5obj, obj, 0)) {
    Py_DECREF(obj);
    Py_DECREF(md5obj);
    return NULL;
  }
  Py_DECREF(obj);
  if (!get_intval(encrypt, "P", &p)) {
    Py_DECREF(md5obj);
    return NULL;
  }
  passbuf[0] = p & 255;
  passbuf[1] = (p >> 8) & 255;
  passbuf[2] = (p >> 16) & 255;
  passbuf[3] = (p >> 24) & 255;
  if (!md5update(md5obj, passbuf, 4)) {
    Py_DECREF(md5obj);
    return NULL;
  }
  if (!(obj = PyMapping_GetItemString(self->trailer, "ID"))) {
    Py_DECREF(md5obj);
    return NULL;
  }
  if (!(idobj = PySequence_GetItem(obj, 0))) {
    Py_DECREF(obj);
    Py_DECREF(md5obj);
    return NULL;
  }
  Py_DECREF(obj);
  if (!md5update_str(md5obj, idobj, 0)) {
    Py_DECREF(idobj);
    Py_DECREF(md5obj);
    return NULL;
  }
  obj = PyObject_CallMethod(md5obj, "digest", NULL);
  Py_DECREF(md5obj);
  if (!obj) {
    Py_DECREF(idobj);
    return NULL;
  }
  if (rev >= 3) {
    for (len = 0; len < 50; len++) {
      if (!(md5obj = PyObject_CallObject(md5, NULL))) {
        Py_DECREF(idobj);
        Py_DECREF(obj);
        return NULL;
      }
      if (!md5update_str(md5obj, obj, keylen)) {
        Py_DECREF(idobj);
        Py_DECREF(obj);
        Py_DECREF(md5obj);
        return NULL;
      }
      Py_DECREF(obj);
      obj = PyObject_CallMethod(md5obj, "digest", NULL);
      Py_DECREF(md5obj);
      if (!obj) {
        Py_DECREF(idobj);
        return NULL;
      }
    }
  }
  obj2 = PySequence_GetSlice(obj, 0, keylen);
  Py_DECREF(obj);
  keystr = PyBytes_AS_STRING(obj2);
  if (rev >= 3) {
    int i;
    if (!(md5obj = PyObject_CallObject(md5, NULL))) {
      Py_DECREF(idobj);
      Py_DECREF(obj2);
      return NULL;
    }
    if (!md5update(md5obj, decrypt_padding, 32) ||
        !md5update_str(md5obj, idobj, 0)) {
      Py_DECREF(md5obj);
      Py_DECREF(idobj);
      Py_DECREF(obj2);
      return NULL;
    }
    obj = PyObject_CallMethod(md5obj, "digest", NULL);
    Py_DECREF(md5obj);
    if (!obj) {
      Py_DECREF(idobj);
      Py_DECREF(obj2);
      return NULL;
    }
    memcpy(passbuf, PyBytes_AS_STRING(obj), 16);
    rc4(keystr, PyBytes_GET_SIZE(obj2), passbuf, 16);
    Py_DECREF(obj);
    for (i = 1; i <= 19; i++) {
      char keybuf[32];
      for (len = 0; len < keylen; len++)
        keybuf[len] = keystr[len] ^ i;
      rc4(keybuf, keylen, passbuf, 16);
    }
    ulen = 16;
  } else {
    memcpy(passbuf, decrypt_padding, 32);
    rc4(keystr, PyBytes_GET_SIZE(obj2), passbuf, 32);
    ulen = 32;
  }
  Py_DECREF(idobj);
  if (!(obj = PyMapping_GetItemString(encrypt, "U"))) {
    Py_DECREF(obj2);
    return NULL;
  }
  if (obj->ob_type == &PDFStringType) {
    ok = PyBytes_GET_SIZE(((PDFString *)obj)->raw) >= ulen &&
        !memcmp(PyBytes_AS_STRING(((PDFString *)obj)->raw), passbuf, ulen);
  } else if (PyBytes_Check(obj)) {
    ok = PyBytes_GET_SIZE(obj) >= ulen &&
      !memcmp(PyBytes_AS_STRING(obj), passbuf, ulen);
  } else {
    pdf_error(self, NULL, "Encrypt U value is a %s not a string",
        obj->ob_type->tp_name);
    Py_DECREF(obj2);
    Py_DECREF(obj);
    return NULL;
  }
  Py_DECREF(obj);
  if (!ok) {
    Py_DECREF(obj2);
    PyErr_SetString(badpassworderror, "Incorrect password");
    return NULL;
  }
  return obj2;
}


static int decrypt(PDF *self, PyObject *password) {
  PyObject *encrypt;
  PyObject *value;
  PyObject *obj;
  long v;
  int cmp;

  if (!PyMapping_HasKeyString(self->trailer, "Encrypt"))
    return 1;
  if (!(encrypt = PyMapping_GetItemString(self->trailer, "Encrypt")))
    return 0;
  if (encrypt->ob_type != &DictionaryType) {
    pdf_error(self, NULL, "Trailer Encrypt entry is a %s not a dictionary",
        encrypt->ob_type->tp_name);
    Py_DECREF(encrypt);
    return 0;
  }
  if (!(value = PyMapping_GetItemString(encrypt, "Filter"))) {
    Py_DECREF(encrypt);
    return 0;
  }
  if (!PyBytes_Check(value) && !PyUnicode_Check(value)) {
    pdf_error(self, NULL, "Trailer Filter value is a %s not a string",
        value->ob_type->tp_name);
    Py_DECREF(value);
    Py_DECREF(encrypt);
    return 0;
  }
  if (!(obj = PyUnicode_DecodeASCII("Standard", 8, "strict"))) {
    Py_DECREF(value);
    Py_DECREF(encrypt);
    return 0;
  }
  cmp = PyObject_RichCompareBool(value, obj, Py_EQ);
  Py_DECREF(obj);
  if (cmp <= 0) {
    if (cmp == 0)
      PyErr_Format(notsupportederror, "Unsupported Encryption Filter '%s'",
          PyBytes_AsString(value));
    Py_DECREF(value);
    Py_DECREF(encrypt);
    return 0;
  }
  Py_DECREF(value);
  if (!(value = PyMapping_GetItemString(encrypt, "V"))) {
    Py_DECREF(encrypt);
    return 0;
  }
  if (!PyLong_Check(value)) {
    pdf_error(self, NULL, "Encrypt V value is a %s not a number",
        value->ob_type->tp_name);
    Py_DECREF(value);
    Py_DECREF(encrypt);
    return 0;
  }
  v = PyLong_AS_LONG(value);
  Py_DECREF(value);
  if (v != 1 && v != 2) {
    PyErr_Format(notsupportederror, "Encrypt V value of %ld is not supported",
        v);
    Py_DECREF(encrypt);
    return 0;
  }
  if (!(self->key = check_password(self, encrypt, password))) {
    Py_DECREF(encrypt);
    return 0;
  }
  Py_DECREF(encrypt);
  return 1;
}


static PyObject *pdf_new(PyTypeObject *type, PyObject *args,
    PyObject *kwargs) {
  PDF *self;
  PyObject *source;
  PyObject *password = NULL;
  int success;

  if (!PyArg_ParseTuple(args, "O|O:PDF", &source, &password))
    return NULL;
  if (!(self = (PDF *)type->tp_alloc(type, 0)))
    return NULL;
  self->weakreflist = NULL;
  self->source = NULL;
  self->version = NULL;
  self->xref = NULL;
  self->trailer = NULL;
  self->key = NULL;
  self->catalog = NULL;
  self->linearized = NULL;
  self->info = NULL;
  self->pages = NULL;
  self->data.buf = NULL;
  Py_INCREF(source);
  self->source = source;

  if (PyObject_GetBuffer(source, &self->data, PyBUF_SIMPLE)) {
    Py_DECREF(self);
    return NULL;
  }

  if (!(self->xref = PyDict_New()) || !(self->trailer =
        PyObject_CallObject((PyObject *)&DictionaryType, NULL))) {
    PyBuffer_Release(&self->data);
    Py_DECREF(self);
    return NULL;
  }

  success = initial_parse(self, (const char *)self->data.buf,
      ((const char *)self->data.buf) + self->data.len);

  PyBuffer_Release(&self->data);
  self->data.buf = NULL;

  if (!success || !decrypt(self, password)) {
    Py_DECREF(self);
    return NULL;
  }

  if (!(self->catalog = PyMapping_GetItemString(self->trailer, "Root"))) {
    Py_DECREF(self);
    return NULL;
  }
  if (PyMapping_HasKeyString(self->trailer, "Info")) {
    if (!(self->info = PyMapping_GetItemString(self->trailer, "Info"))) {
      Py_DECREF(self);
      return NULL;
    }
  }
  if (PyMapping_HasKeyString(self->catalog, "Version")) {
    Py_CLEAR(self->version);
    if (!(self->version = PyMapping_GetItemString(
            self->catalog, "Version"))) {
      Py_DECREF(self);
      return NULL;
    }
  }

  return (PyObject *)self;
}


PyObject *ascii85decode(PDF *self, const char *data, Py_ssize_t len,
    PyObject *parms, long *usedbytes) {
  PyObject *result;
  Py_ssize_t oplen;
  const char *cp;
  char *output;
  char *op;
  char i;

  output = NULL;
  oplen = 0;
  while (1) {
    long word;
    char index;
    cp = data;
    op = output;
    index = 0;
    word = 0;
    while (cp < data + len) {
      if (*cp >= '!' && *cp <= 'u') {
        word = (word * 85) + (*(cp++) - '!');
        index++;
        if (index == 5) {
          if (op) {
            *(op++) = (word >> 24) & 255;
            *(op++) = (word >> 16) & 255;
            *(op++) = (word >> 8) & 255;
            *(op++) = word & 255;
          } else {
            oplen += 4;
          }
          index = 0;
          word = 0;
        }
      } else if (*cp == 'z' && index == 0) {
        cp++;
        if (op) {
          *(op++) = 0;
          *(op++) = 0;
          *(op++) = 0;
          *(op++) = 0;
        } else {
          oplen += 4;
        }
      } else if (cp == data && cp + 1 < data + len &&
          *cp == '<' && cp[1] == '~') {
        cp += 2;
      } else if (cp + 1 < data + len && *cp == '~' && cp[1] == '>') {
        cp += 2;
        break;
      } else if (is_pdfwhitespace(*cp)) {
        cp++;
      } else {
        if (output)
          PyMem_Free(output);
        PyErr_SetString(PyExc_ValueError, "Bad character in ASCII85 section");
        return NULL;
      }
    }
    if (index == 1) {
      if (output)
        PyMem_Free(output);
      PyErr_SetString(PyExc_ValueError,
          "Trailing character in ASCII85 section.");
      return NULL;
    }
    if (index >= 2) {
      for (i = index; i < 5; i++)
        word = (word * 85) + 84;
      if (op) {
        *(op++) = (word >> 24) & 255;
        if (index >= 3)
          *(op++) = (word >> 16) & 255;
        if (index >= 4)
          *(op++) = (word >> 8) & 255;
      } else {
        oplen += index - 1;
      }
    }
    if (output)
      break;
    if (!(output = PyMem_Malloc(oplen)))
      return PyErr_NoMemory();
  }

  result = PyBytes_FromStringAndSize(output, op - output);
  PyMem_Free(output);
  if (usedbytes)
    *usedbytes = cp - data;
  return result;
}


PyObject *asciihexdecode(PDF *self, const char *data, Py_ssize_t len,
    PyObject *parms, long *usedbytes) {
  PyObject *result;
  const char *cp;
  const char *end;
  char *output;
  char *op;

  if (!(output = PyMem_Malloc(1 + len / 2)))
    return PyErr_NoMemory();
  cp = data;
  end = data + len;
  op = output;
  while (cp < end) {
    char digit;
    cp = skipwhitespace(cp, end);
    if (cp >= end)
      break;
    if (*cp == '>') {
      cp++;
      break;
    }
    if ((digit = decodexdigit(*(cp++))) == 16) {
      PyMem_Free(output);
      PyErr_SetString(PyExc_ValueError,
          "Invalid character in ASCIIHex string");
      return NULL;
    }
    *(op++) = digit << 4;
    cp = skipwhitespace(cp, end);
    if (cp >= end)
      break;
    if (*cp == '>') {
      cp++;
      break;
    }
    if ((digit = decodexdigit(*(cp++))) == 16) {
      PyMem_Free(output);
      PyErr_SetString(PyExc_ValueError,
          "Invalid character in ASCIIHex string");
      return NULL;
    }
    op[-1] |= digit;
  }

  result = PyBytes_FromStringAndSize(output, op - output);
  PyMem_Free(output);
  if (usedbytes)
    *usedbytes = cp - data;
  return result;
}


PyObject *do_predictor(PyObject *input, PyObject *parms) {
  long predictor = 1;
  long colors = 1;
  long bitspercomponent = 8;
  long columns = 1;
  int bytesperpixel;
  int bytesperrow;
  int i;
  int a, b, c, p;
  unsigned char *data;
  Py_ssize_t len;
  const unsigned char *cp;
  unsigned char *output;
  unsigned char *op;
  unsigned char *thisrow;
  unsigned char *prevrow;
  PyObject *result;

  if (parms != Py_None && PyMapping_HasKeyString(parms, "Predictor"))
    if (!get_intval(parms, "Predictor", &predictor))
      return NULL;

  if (predictor == 1) {
    Py_INCREF(input);
    return input;
  }

  if (predictor != 2 && (predictor < 10 || predictor > 15)) {
    PyErr_Format(PyExc_ValueError,
        "Invalid DecodeParms Predictor value of %ld", predictor);
    return NULL;
  }
  if (predictor == 2) {
    PyErr_SetString(notsupportederror,
        "DecodeParms Predictor value of 2 not supported");
    return NULL;
  }

  if (PyMapping_HasKeyString(parms, "Colors"))
    if (!get_intval(parms, "Colors", &colors))
      return NULL;
  if (PyMapping_HasKeyString(parms, "BitsPerComponent"))
    if (!get_intval(parms, "BitsPerComponent", &bitspercomponent))
      return NULL;
  if (PyMapping_HasKeyString(parms, "Columns"))
    if (!get_intval(parms, "Columns", &columns))
      return NULL;

  if (PyBytes_AsStringAndSize(input, (char **)&data, &len) < 0)
    return NULL;
  if (!(output = PyMem_Malloc(len)))
    return PyErr_NoMemory();

  bytesperrow = 1 + (colors * bitspercomponent * columns + 7) / 8;
  bytesperpixel = (colors * bitspercomponent + 7) / 8;
  op = output;
  prevrow = NULL;
  for (cp = data; cp + bytesperrow <= data + len; cp += bytesperrow) {
    thisrow = op;
    switch (*cp) {
      case 0: /* no predictor */
        for (i = 1; i < bytesperrow; i++)
          *(op++) = cp[i];
        break;
      case 1: /* compare with value to the left */
        for (i = 1; i < bytesperrow; i++)
          *(op++) = cp[i] +
            (i > bytesperpixel ? thisrow[i - 1 - bytesperpixel] : 0);
        break;
      case 2: /* compare with value in the row above */
        for (i = 1; i < bytesperrow; i++)
          *(op++) = cp[i] + (prevrow ? prevrow[i - 1] : 0);
        break;
      case 3: /* compare with the average of the left and above values */
        for (i = 1; i < bytesperrow; i++)
          *(op++) = cp[i] + (((unsigned int)(i > bytesperpixel ?
                  thisrow[i - 1 - bytesperpixel] : 0) +
                (prevrow ? prevrow[i - 1] : 0)) /  2);
        break;
      case 4: /* "Paeth" method */
        for (i = 1; i < bytesperrow; i++) {
          a = i > bytesperpixel ? thisrow[i - 1 - bytesperpixel] : 0;
          b = prevrow ? prevrow[i - 1] : 0;
          c = (i > bytesperpixel && prevrow) ?
            prevrow[i - 1 - bytesperpixel] : 0;
          p = a + b - c;
          if ((a > p ? a - p : p - a) <= (b > p ? b - p : p - b) &&
            (a > p ? a - p : p - a) <= (c > p ? c - p : p - c))
            *(op++) = cp[i] + a;
          else if ((b > p ? b - p : p - b) <= (c > p ? c - p : p - c))
            *(op++) = cp[i] + b;
          else
            *(op++) = cp[i] + c;
        }
        break;
      default:
        PyMem_Free(output);
        PyErr_Format(PyExc_ValueError,
            "Unknown PNG Predictor filter type %d", (int)*cp);
        return NULL;
    }
    prevrow = thisrow;
  }

  result = PyBytes_FromStringAndSize((char *)output, op - output);
  PyMem_Free(output);
  return result;
}


PyObject *flatedecode(PDF *self, const char *data, Py_ssize_t len,
    PyObject *parms, long *usedbytes) {
  PyObject *decompress;
  PyObject *obj;
  PyObject *obj2;
  Py_ssize_t unused_data;

  if (!(decompress = PyObject_CallFunctionObjArgs(decompressobj, NULL)))
    return NULL;
  if (!(obj = PyObject_CallMethod(decompress, "decompress", FORMAT_BYTES,
          data, len))) {
    Py_DECREF(decompress);
    return NULL;
  }
  if (!(obj2 = PyObject_GetAttrString(decompress, "unused_data"))) {
    Py_DECREF(obj);
    Py_DECREF(decompress);
    return NULL;
  }
  unused_data = PyObject_Length(obj2);
  Py_DECREF(obj2);
  Py_DECREF(decompress);
  if (unused_data < 0) {
    Py_DECREF(obj);
    return NULL;
  }
  obj2 = do_predictor(obj, parms);
  Py_DECREF(obj);
  if (usedbytes)
    *usedbytes = len - unused_data;
  return obj2;
}


static filterdecoder filterdecoders[] = {
  { "ASCII85Decode", ascii85decode },
  { "ASCIIHexDecode", asciihexdecode },
  { "LZWDecode", NULL },
  { "FlateDecode", flatedecode },
  { "RunLengthDecode", NULL },
  { "CCITTFaxDecode", NULL },
  { "JBIG2Decode", NULL },
  { "DCTDecode", NULL },
  { "JPXDecode", NULL },
  { "Crypt", NULL },
  { NULL }
};


static const char *inheritable_page_keys[] = {
  "Resources",
  "MediaBox",
  "CropBox",
  "Rotate",
  NULL
};


static int flatten_pages(PDF *self, PyObject *this, PyObject *parent) {
  PyObject *obj;
  PyObject *iter;

  if (parent) {
    char **key;
    for (key = (char **)inheritable_page_keys; *key; key++) {
      if (!PyMapping_HasKeyString(this, *key) &&
          PyMapping_HasKeyString(parent, *key)) {
        if (!(obj = PyMapping_GetItemString(parent, *key)))
          return 0;
        if (PyMapping_SetItemString(this, *key, obj) < 0) {
          Py_DECREF(obj);
          return 0;
        }
        Py_DECREF(obj);
      }
    }
  }
  if (this->ob_type == &PageType) {
    if (PyList_Append(self->pages, this) < 0)
      return 0;
    return 1;
  }
  if (!PyMapping_HasKeyString(this, "Kids"))
    return 1;
  if (!(obj = PyMapping_GetItemString(this, "Kids")))
    return 0;
  if (obj->ob_type != &ArrayType) {
    pdf_error(self, NULL, "Page Tree Kids entry is a %s not an array",
        obj->ob_type->tp_name);
    Py_DECREF(obj);
    return 0;
  }
  iter = PyObject_GetIter(obj);
  Py_DECREF(obj);
  if (!iter)
    return 0;
  while ((obj = PyIter_Next(iter))) {
    if (obj->ob_type != &DictionaryType && obj->ob_type != &PageType) {
      pdf_error(self, NULL, "Page Tree entry is a %s not a dictionary",
          obj->ob_type->tp_name);
      Py_DECREF(obj);
      Py_DECREF(iter);
      return 0;
    }
    if (!flatten_pages(self, obj, this)) {
      Py_DECREF(obj);
      Py_DECREF(iter);
      return 0;
    }
    Py_DECREF(obj);
  }
  Py_DECREF(iter);
  return !PyErr_Occurred();
}


static PyObject *pdf_get_pages(PDF *self, void *closure) {
  if (!self->pages) {
    PyObject *obj;
    if (!(obj = PyMapping_GetItemString(self->catalog, "Pages")))
      return NULL;
    if (obj->ob_type != &DictionaryType) {
      pdf_error(self, NULL, "Catalog Pages entry is a %s not a dictionary",
          obj->ob_type->tp_name);
      Py_DECREF(obj);
      return NULL;
    }
    if (!(self->pages = PyList_New(0))) {
      Py_DECREF(obj);
      return NULL;
    }
    if (!flatten_pages(self, obj, NULL)) {
      Py_CLEAR(self->pages);
      return NULL;
    }
    Py_DECREF(obj);
  }
  Py_INCREF(self->pages);
  return self->pages;
}


static PyMemberDef pdf_members[] = {
  { "version", T_OBJECT, offsetof(PDF, version), READONLY,
    "File format version" },
  { "xref", T_OBJECT, offsetof(PDF, xref), READONLY,
    "Cross-reference table" },
  { "trailer", T_OBJECT, offsetof(PDF, trailer), READONLY,
    "File trailer" },
  { "catalog", T_OBJECT, offsetof(PDF, catalog), READONLY,
    "Document catalog" },
  { "info", T_OBJECT, offsetof(PDF, info), READONLY,
    "Document information dictionary" },
  { "linearized", T_OBJECT, offsetof(PDF, linearized), READONLY,
    "Linearization parameter dictionary" },
  { "key", T_OBJECT, offsetof(PDF, key), READONLY,
    "Decryption key" },
  { NULL }
};


static PyGetSetDef pdf_getset[] = {
  { "pages", (getter)pdf_get_pages, NULL,
    "The flattened array of page objects", NULL },
  { NULL }
};


static PyMethodDef pdf_methods[] = {
  { NULL }
};


static PyTypeObject PDFType = {
  PyVarObject_HEAD_INIT(NULL, 0)
  "pycpdf.PDF",               /*tp_name*/
  sizeof(PDF),                /*tp_basicsize*/
  0,                          /*tp_itemsize*/
  (destructor)pdf_dealloc,    /*tp_dealloc*/
  0,                          /*tp_print*/
  0,                          /*tp_getattr*/
  0,                          /*tp_setattr*/
  0,                          /*tp_compare*/
  0,                          /*tp_repr*/
  0,                          /*tp_as_number*/
  0,                          /*tp_as_sequence*/
  0,                          /*tp_as_mapping*/
  0,                          /*tp_hash*/
  0,                          /*tp_call*/
  0,                          /*tp_str*/
  0,                          /*tp_getattro*/
  0,                          /*tp_setattro*/
  0,                          /*tp_as_buffer*/
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_GC, /*tp_flags*/
  "PDF objects",              /*tp_doc*/
  (traverseproc)pdf_traverse, /*tp_traverse*/
  (inquiry)pdf_clear,         /*tp_clear*/
  0,                          /*tp_richcompare*/
  offsetof(PDF, weakreflist), /*tp_weaklistoffset*/
  0,                          /*tp_iter*/
  0,                          /*tp_iternext*/
  pdf_methods,                /*tp_methods*/
  pdf_members,                /*tp_members*/
  pdf_getset,                 /*tp_getset*/
  0,                          /*tp_base*/
  0,                          /*tp_dict*/
  0,                          /*tp_descr_get*/
  0,                          /*tp_descr_set*/
  0,                          /*tp_dictoffset*/
  0,                          /*tp_init*/
  0,                          /*tp_alloc*/
  pdf_new,                    /*tp_new*/
  0,                          /*tp_free*/
  0,                          /*tp_is_gc*/
};


static PyObject *pycpdf_refs(PyObject *self, PyObject *args) {
  PyObject *obj;

  if (!PyArg_ParseTuple(args, "O:refs", &obj))
    return NULL;
  return PyLong_FromLong((long)obj->ob_refcnt - 1);
}


static PyMethodDef pycpdf_methods[] = {
  { "refs", (PyCFunction)pycpdf_refs, METH_VARARGS, "Count refs" },
  { NULL }
};


PyDoc_STRVAR(pycpdf_doc,
    "Provides fast read-only access to information"
    " contained within PDF files");


static const char *raw_standardencoding =
  "\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff"
  "\xfe\xff\x09\x00\x0a\x00\xfe\xff\xfe\xff\x0d\x00\xfe\xff\xfe\xff"
  "\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff"
  "\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff"
  "\x20\x00\x21\x00\x22\x00\x23\x00\x24\x00\x25\x00\x26\x00\x19\x20"
  "\x28\x00\x29\x00\x2a\x00\x2b\x00\x2c\x00\x2d\x00\x2e\x00\x2f\x00"
  "\x30\x00\x31\x00\x32\x00\x33\x00\x34\x00\x35\x00\x36\x00\x37\x00"
  "\x38\x00\x39\x00\x3a\x00\x3b\x00\x3c\x00\x3d\x00\x3e\x00\x3f\x00"
  "\x40\x00\x41\x00\x42\x00\x43\x00\x44\x00\x45\x00\x46\x00\x47\x00"
  "\x48\x00\x49\x00\x4a\x00\x4b\x00\x4c\x00\x4d\x00\x4e\x00\x4f\x00"
  "\x50\x00\x51\x00\x52\x00\x53\x00\x54\x00\x55\x00\x56\x00\x57\x00"
  "\x58\x00\x59\x00\x5a\x00\x5b\x00\x5c\x00\x5d\x00\x5e\x00\x5f\x00"
  "\x18\x20\x61\x00\x62\x00\x63\x00\x64\x00\x65\x00\x66\x00\x67\x00"
  "\x68\x00\x69\x00\x6a\x00\x6b\x00\x6c\x00\x6d\x00\x6e\x00\x6f\x00"
  "\x70\x00\x71\x00\x72\x00\x73\x00\x74\x00\x75\x00\x76\x00\x77\x00"
  "\x78\x00\x79\x00\x7a\x00\x7b\x00\x7c\x00\x7d\x00\x7e\x00\xfe\xff"
  "\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff"
  "\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff"
  "\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff"
  "\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff"
  "\xfe\xff\xa1\x00\xa2\x00\xa3\x00\x44\x20\xa5\x00\x92\x01\xa7\x00"
  "\xa4\x00\x27\x00\x1c\x20\xab\x00\x39\x20\x3a\x20\x01\xfb\x02\xfb"
  "\xfe\xff\x13\x20\x20\x20\x21\x20\xb7\x00\xfe\xff\xb6\x00\x22\x20"
  "\x1a\x20\x1e\x20\x1d\x20\xbb\x00\x26\x20\x30\x20\xfe\xff\xbf\x00"
  "\xfe\xff\x60\x00\xb4\x00\xc6\x02\xdc\x02\xaf\x00\xd8\x02\xd9\x02"
  "\xa8\x00\xfe\xff\xda\x02\xb8\x00\xfe\xff\xdd\x02\xdb\x02\xc7\x02"
  "\x14\x20\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff"
  "\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff"
  "\xfe\xff\xc6\x00\xfe\xff\xaa\x00\xfe\xff\xfe\xff\xfe\xff\xfe\xff"
  "\x41\x01\xd8\x00\x52\x01\xba\x00\xfe\xff\xfe\xff\xfe\xff\xfe\xff"
  "\xfe\xff\xe6\x00\xfe\xff\xfe\xff\xfe\xff\x31\x01\xfe\xff\xfe\xff"
  "\x42\x01\xf8\x00\x53\x01\xdf\x00\xfe\xff\xfe\xff\xfe\xff\xfe\xff";


static const char *raw_macromanencoding =
  "\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff"
  "\xfe\xff\x09\x00\x0a\x00\xfe\xff\xfe\xff\x0d\x00\xfe\xff\xfe\xff"
  "\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff"
  "\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff"
  "\x20\x00\x21\x00\x22\x00\x23\x00\x24\x00\x25\x00\x26\x00\x27\x00"
  "\x28\x00\x29\x00\x2a\x00\x2b\x00\x2c\x00\x2d\x00\x2e\x00\x2f\x00"
  "\x30\x00\x31\x00\x32\x00\x33\x00\x34\x00\x35\x00\x36\x00\x37\x00"
  "\x38\x00\x39\x00\x3a\x00\x3b\x00\x3c\x00\x3d\x00\x3e\x00\x3f\x00"
  "\x40\x00\x41\x00\x42\x00\x43\x00\x44\x00\x45\x00\x46\x00\x47\x00"
  "\x48\x00\x49\x00\x4a\x00\x4b\x00\x4c\x00\x4d\x00\x4e\x00\x4f\x00"
  "\x50\x00\x51\x00\x52\x00\x53\x00\x54\x00\x55\x00\x56\x00\x57\x00"
  "\x58\x00\x59\x00\x5a\x00\x5b\x00\x5c\x00\x5d\x00\x5e\x00\x5f\x00"
  "\x60\x00\x61\x00\x62\x00\x63\x00\x64\x00\x65\x00\x66\x00\x67\x00"
  "\x68\x00\x69\x00\x6a\x00\x6b\x00\x6c\x00\x6d\x00\x6e\x00\x6f\x00"
  "\x70\x00\x71\x00\x72\x00\x73\x00\x74\x00\x75\x00\x76\x00\x77\x00"
  "\x78\x00\x79\x00\x7a\x00\x7b\x00\x7c\x00\x7d\x00\x7e\x00\xfe\xff"
  "\xc4\x00\xc5\x00\xc7\x00\xc9\x00\xd1\x00\xd6\x00\xdc\x00\xe1\x00"
  "\xe0\x00\xe2\x00\xe4\x00\xe3\x00\xe5\x00\xe7\x00\xe9\x00\xe8\x00"
  "\xea\x00\xeb\x00\xed\x00\xec\x00\xee\x00\xef\x00\xf1\x00\xf3\x00"
  "\xf2\x00\xf4\x00\xf6\x00\xf5\x00\xfa\x00\xf9\x00\xfb\x00\xfc\x00"
  "\x20\x20\xb0\x00\xa2\x00\xa3\x00\xa7\x00\x22\x20\xb6\x00\xdf\x00"
  "\xae\x00\xa9\x00\x22\x21\xb4\x00\xa8\x00\xfe\xff\xc6\x00\xd8\x00"
  "\xfe\xff\xb1\x00\xfe\xff\xfe\xff\xa5\x00\xb5\x00\xfe\xff\xfe\xff"
  "\xfe\xff\xfe\xff\xfe\xff\xaa\x00\xba\x00\xfe\xff\xe6\x00\xf8\x00"
  "\xbf\x00\xa1\x00\xac\x00\xfe\xff\x92\x01\xfe\xff\xfe\xff\xab\x00"
  "\xbb\x00\x26\x20\x20\x00\xc0\x00\xc3\x00\xd5\x00\x52\x01\x53\x01"
  "\x13\x20\x14\x20\x1c\x20\x1d\x20\x18\x20\x19\x20\xf7\x00\xfe\xff"
  "\xff\x00\x78\x01\x44\x20\xa4\x00\x39\x20\x3a\x20\x01\xfb\x02\xfb"
  "\x21\x20\xb7\x00\x1a\x20\x1e\x20\x30\x20\xc2\x00\xca\x00\xc1\x00"
  "\xcb\x00\xc8\x00\xcd\x00\xce\x00\xcf\x00\xcc\x00\xd3\x00\xd4\x00"
  "\xfe\xff\xd2\x00\xda\x00\xdb\x00\xd9\x00\x31\x01\xc6\x02\xdc\x02"
  "\xaf\x00\xd8\x02\xd9\x02\xda\x02\xb8\x00\xdd\x02\xdb\x02\xc7\x02";


static const char *raw_winansiencoding =
  "\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff"
  "\xfe\xff\x09\x00\x0a\x00\xfe\xff\xfe\xff\x0d\x00\xfe\xff\xfe\xff"
  "\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff"
  "\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff"
  "\x20\x00\x21\x00\x22\x00\x23\x00\x24\x00\x25\x00\x26\x00\x27\x00"
  "\x28\x00\x29\x00\x2a\x00\x2b\x00\x2c\x00\x2d\x00\x2e\x00\x2f\x00"
  "\x30\x00\x31\x00\x32\x00\x33\x00\x34\x00\x35\x00\x36\x00\x37\x00"
  "\x38\x00\x39\x00\x3a\x00\x3b\x00\x3c\x00\x3d\x00\x3e\x00\x3f\x00"
  "\x40\x00\x41\x00\x42\x00\x43\x00\x44\x00\x45\x00\x46\x00\x47\x00"
  "\x48\x00\x49\x00\x4a\x00\x4b\x00\x4c\x00\x4d\x00\x4e\x00\x4f\x00"
  "\x50\x00\x51\x00\x52\x00\x53\x00\x54\x00\x55\x00\x56\x00\x57\x00"
  "\x58\x00\x59\x00\x5a\x00\x5b\x00\x5c\x00\x5d\x00\x5e\x00\x5f\x00"
  "\x60\x00\x61\x00\x62\x00\x63\x00\x64\x00\x65\x00\x66\x00\x67\x00"
  "\x68\x00\x69\x00\x6a\x00\x6b\x00\x6c\x00\x6d\x00\x6e\x00\x6f\x00"
  "\x70\x00\x71\x00\x72\x00\x73\x00\x74\x00\x75\x00\x76\x00\x77\x00"
  "\x78\x00\x79\x00\x7a\x00\x7b\x00\x7c\x00\x7d\x00\x7e\x00\x22\x20"
  "\xac\x20\x22\x20\x1a\x20\x92\x01\x1e\x20\x26\x20\x20\x20\x21\x20"
  "\xc6\x02\x30\x20\x60\x01\x39\x20\x52\x01\x22\x20\x7d\x01\x22\x20"
  "\x22\x20\x18\x20\x19\x20\x1c\x20\x1d\x20\x22\x20\x13\x20\x14\x20"
  "\xdc\x02\x22\x21\x61\x01\x3a\x20\x53\x01\x22\x20\x7e\x01\x78\x01"
  "\x20\x00\xa1\x00\xa2\x00\xa3\x00\xa4\x00\xa5\x00\xa6\x00\xa7\x00"
  "\xa8\x00\xa9\x00\xaa\x00\xab\x00\xac\x00\x2d\x00\xae\x00\xaf\x00"
  "\xb0\x00\xb1\x00\xb2\x00\xb3\x00\xb4\x00\xb5\x00\xb6\x00\xb7\x00"
  "\xb8\x00\xb9\x00\xba\x00\xbb\x00\xbc\x00\xbd\x00\xbe\x00\xbf\x00"
  "\xc0\x00\xc1\x00\xc2\x00\xc3\x00\xc4\x00\xc5\x00\xc6\x00\xc7\x00"
  "\xc8\x00\xc9\x00\xca\x00\xcb\x00\xcc\x00\xcd\x00\xce\x00\xcf\x00"
  "\xd0\x00\xd1\x00\xd2\x00\xd3\x00\xd4\x00\xd5\x00\xd6\x00\xd7\x00"
  "\xd8\x00\xd9\x00\xda\x00\xdb\x00\xdc\x00\xdd\x00\xde\x00\xdf\x00"
  "\xe0\x00\xe1\x00\xe2\x00\xe3\x00\xe4\x00\xe5\x00\xe6\x00\xe7\x00"
  "\xe8\x00\xe9\x00\xea\x00\xeb\x00\xec\x00\xed\x00\xee\x00\xef\x00"
  "\xf0\x00\xf1\x00\xf2\x00\xf3\x00\xf4\x00\xf5\x00\xf6\x00\xf7\x00"
  "\xf8\x00\xf9\x00\xfa\x00\xfb\x00\xfc\x00\xfd\x00\xfe\x00\xff\x00";


static const char *raw_pdfdocencoding =
  "\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff"
  "\xfe\xff\x09\x00\x0a\x00\xfe\xff\xfe\xff\x0d\x00\xfe\xff\xfe\xff"
  "\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff"
  "\xd8\x02\xc7\x02\xc6\x02\xd9\x02\xdd\x02\xdb\x02\xda\x02\xdc\x02"
  "\x20\x00\x21\x00\x22\x00\x23\x00\x24\x00\x25\x00\x26\x00\x27\x00"
  "\x28\x00\x29\x00\x2a\x00\x2b\x00\x2c\x00\x2d\x00\x2e\x00\x2f\x00"
  "\x30\x00\x31\x00\x32\x00\x33\x00\x34\x00\x35\x00\x36\x00\x37\x00"
  "\x38\x00\x39\x00\x3a\x00\x3b\x00\x3c\x00\x3d\x00\x3e\x00\x3f\x00"
  "\x40\x00\x41\x00\x42\x00\x43\x00\x44\x00\x45\x00\x46\x00\x47\x00"
  "\x48\x00\x49\x00\x4a\x00\x4b\x00\x4c\x00\x4d\x00\x4e\x00\x4f\x00"
  "\x50\x00\x51\x00\x52\x00\x53\x00\x54\x00\x55\x00\x56\x00\x57\x00"
  "\x58\x00\x59\x00\x5a\x00\x5b\x00\x5c\x00\x5d\x00\x5e\x00\x5f\x00"
  "\x60\x00\x61\x00\x62\x00\x63\x00\x64\x00\x65\x00\x66\x00\x67\x00"
  "\x68\x00\x69\x00\x6a\x00\x6b\x00\x6c\x00\x6d\x00\x6e\x00\x6f\x00"
  "\x70\x00\x71\x00\x72\x00\x73\x00\x74\x00\x75\x00\x76\x00\x77\x00"
  "\x78\x00\x79\x00\x7a\x00\x7b\x00\x7c\x00\x7d\x00\x7e\x00\xfe\xff"
  "\x22\x20\x20\x20\x21\x20\x26\x20\x14\x20\x13\x20\x92\x01\x44\x20"
  "\x39\x20\x3a\x20\x12\x22\x30\x20\x1e\x20\x1c\x20\x1d\x20\x18\x20"
  "\x19\x20\x1a\x20\x22\x21\x01\xfb\x02\xfb\x41\x01\x52\x01\x60\x01"
  "\x78\x01\x7d\x01\x31\x01\x42\x01\x53\x01\x61\x01\x7e\x01\xfe\xff"
  "\xac\x20\xa1\x00\xa2\x00\xa3\x00\xa4\x00\xa5\x00\xa6\x00\xa7\x00"
  "\xa8\x00\xa9\x00\xaa\x00\xab\x00\xac\x00\xfe\xff\xae\x00\xaf\x00"
  "\xb0\x00\xb1\x00\xb2\x00\xb3\x00\xb4\x00\xb5\x00\xb6\x00\xb7\x00"
  "\xb8\x00\xb9\x00\xba\x00\xbb\x00\xbc\x00\xbd\x00\xbe\x00\xbf\x00"
  "\xc0\x00\xc1\x00\xc2\x00\xc3\x00\xc4\x00\xc5\x00\xc6\x00\xc7\x00"
  "\xc8\x00\xc9\x00\xca\x00\xcb\x00\xcc\x00\xcd\x00\xce\x00\xcf\x00"
  "\xd0\x00\xd1\x00\xd2\x00\xd3\x00\xd4\x00\xd5\x00\xd6\x00\xd7\x00"
  "\xd8\x00\xd9\x00\xda\x00\xdb\x00\xdc\x00\xdd\x00\xde\x00\xdf\x00"
  "\xe0\x00\xe1\x00\xe2\x00\xe3\x00\xe4\x00\xe5\x00\xe6\x00\xe7\x00"
  "\xe8\x00\xe9\x00\xea\x00\xeb\x00\xec\x00\xed\x00\xee\x00\xef\x00"
  "\xf0\x00\xf1\x00\xf2\x00\xf3\x00\xf4\x00\xf5\x00\xf6\x00\xf7\x00"
  "\xf8\x00\xf9\x00\xfa\x00\xfb\x00\xfc\x00\xfd\x00\xfe\x00\xff\x00";


static const char raw_macexpertencoding[512] = {
  "\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff"
  "\xfe\xff\x09\x00\x0a\x00\xfe\xff\xfe\xff\x0d\x00\xfe\xff\xfe\xff"
  "\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff"
  "\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff"
  "\x20\x00\x21\xf7\xf8\xf6\xa2\xf7\x24\xf7\xe4\xf6\x26\xf7\xb4\xf7"
  "\x7d\x20\x7e\x20\x25\x20\x24\x20\x2c\x00\x2d\x00\x2e\x00\x44\x20"
  "\x30\xf7\x31\xf7\x32\xf7\x33\xf7\x34\xf7\x35\xf7\x36\xf7\x37\xf7"
  "\x38\xf7\x39\xf7\x3a\x00\x3b\x00\xfe\xff\xde\xf6\xfe\xff\x3f\xf7"
  "\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xf0\xf7\xfe\xff\xfe\xff\xbc\x00"
  "\xbd\x00\xbe\x00\x5b\x21\x5c\x21\x5d\x21\x5e\x21\x53\x21\x54\x21"
  "\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\x00\xfb\x01\xfb"
  "\x02\xfb\x03\xfb\x04\xfb\x8d\x20\xfe\xff\x8e\x20\xf6\xf6\xe5\xf6"
  "\x60\xf7\x61\xf7\x62\xf7\x63\xf7\x64\xf7\x65\xf7\x66\xf7\x67\xf7"
  "\x68\xf7\x69\xf7\x6a\xf7\x6b\xf7\x6c\xf7\x6d\xf7\x6e\xf7\x6f\xf7"
  "\x70\xf7\x71\xf7\x72\xf7\x73\xf7\x74\xf7\x75\xf7\x76\xf7\x77\xf7"
  "\x78\xf7\x79\xf7\x7a\xf7\xa1\x20\xdc\xf6\xdd\xf6\xfe\xf6\xfe\xff"
  "\xfe\xff\xe9\xf6\xe0\xf6\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xe1\xf7"
  "\xe0\xf7\xe2\xf7\xe4\xf7\xe3\xf7\xe5\xf7\xe7\xf7\xe9\xf7\xe8\xf7"
  "\xea\xf7\xeb\xf7\xed\xf7\xec\xf7\xee\xf7\xef\xf7\xf1\xf7\xf3\xf7"
  "\xf2\xf7\xf4\xf7\xf6\xf7\xf5\xf7\xfa\xf7\xf9\xf7\xfb\xf7\xfc\xf7"
  "\xfe\xff\x78\x20\x84\x20\x83\x20\x86\x20\x88\x20\x87\x20\xfd\xf6"
  "\xfe\xff\xdf\xf6\x82\x20\xfe\xff\xa8\xf7\xfe\xff\xf5\xf6\xf0\xf6"
  "\x85\x20\xfe\xff\xe1\xf6\xe7\xf6\xfd\xf7\xfe\xff\xe3\xf6\xfe\xff"
  "\xfe\xff\xfe\xf7\xfe\xff\x89\x20\x80\x20\xff\xf6\xe6\xf7\xf8\xf7"
  "\xbf\xf7\x81\x20\xf9\xf6\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff"
  "\xfe\xff\xb8\xf7\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xfa\xf6"
  "\x12\x20\xe6\xf6\xfe\xff\xfe\xff\xfe\xff\xfe\xff\xa1\xf7\xfe\xff"
  "\xff\xf7\xfe\xff\xb9\x00\xb2\x00\xb3\x00\x74\x20\x75\x20\x76\x20"
  "\x77\x20\x79\x20\x70\x20\xfe\xff\xec\xf6\xf1\xf6\xf3\xf6\xfe\xff"
  "\xfe\xff\xed\xf6\xf2\xf6\xeb\xf6\xfe\xff\xfe\xff\xfe\xff\xfe\xff"
  "\xfe\xff\xee\xf6\xfb\xf6\xf4\xf6\xaf\xf7\xea\xf6\x7f\x20\xef\xf6"
  "\xe2\xf6\xe8\xf6\xf7\xf6\xfc\xf6\xfe\xff\xfe\xff\xfe\xff\xfe\xff"
};


static const char *glyph_names[] = {
  "A", "AE", "AEacute", "AEmacron", "AEsmall", "Aacute", "Aacutesmall",
  "Abreve", "Abreveacute", "Abrevecyrillic", "Abrevedotbelow", "Abrevegrave",
  "Abrevehookabove", "Abrevetilde", "Acaron", "Acircle", "Acircumflex",
  "Acircumflexacute", "Acircumflexdotbelow", "Acircumflexgrave",
  "Acircumflexhookabove", "Acircumflexsmall", "Acircumflextilde", "Acute",
  "Acutesmall", "Acyrillic", "Adblgrave", "Adieresis", "Adieresiscyrillic",
  "Adieresismacron", "Adieresissmall", "Adotbelow", "Adotmacron", "Agrave",
  "Agravesmall", "Ahookabove", "Aiecyrillic", "Ainvertedbreve", "Alpha",
  "Alphatonos", "Amacron", "Amonospace", "Aogonek", "Aring", "Aringacute",
  "Aringbelow", "Aringsmall", "Asmall", "Atilde", "Atildesmall",
  "Aybarmenian", "B", "Bcircle", "Bdotaccent", "Bdotbelow", "Becyrillic",
  "Benarmenian", "Beta", "Bhook", "Blinebelow", "Bmonospace", "Brevesmall",
  "Bsmall", "Btopbar", "C", "Caarmenian", "Cacute", "Caron", "Caronsmall",
  "Ccaron", "Ccedilla", "Ccedillaacute", "Ccedillasmall", "Ccircle",
  "Ccircumflex", "Cdot", "Cdotaccent", "Cedillasmall", "Chaarmenian",
  "Cheabkhasiancyrillic", "Checyrillic", "Chedescenderabkhasiancyrillic",
  "Chedescendercyrillic", "Chedieresiscyrillic", "Cheharmenian",
  "Chekhakassiancyrillic", "Cheverticalstrokecyrillic", "Chi", "Chook",
  "Circumflexsmall", "Cmonospace", "Coarmenian", "Csmall", "D", "DZ",
  "DZcaron", "Daarmenian", "Dafrican", "Dcaron", "Dcedilla", "Dcircle",
  "Dcircumflexbelow", "Dcroat", "Ddotaccent", "Ddotbelow", "Decyrillic",
  "Deicoptic", "Delta", "Deltagreek", "Dhook", "Dieresis", "DieresisAcute",
  "DieresisGrave", "Dieresissmall", "Digammagreek", "Djecyrillic",
  "Dlinebelow", "Dmonospace", "Dotaccentsmall", "Dslash", "Dsmall", "Dtopbar",
  "Dz", "Dzcaron", "Dzeabkhasiancyrillic", "Dzecyrillic", "Dzhecyrillic", "E",
  "Eacute", "Eacutesmall", "Ebreve", "Ecaron", "Ecedillabreve", "Echarmenian",
  "Ecircle", "Ecircumflex", "Ecircumflexacute", "Ecircumflexbelow",
  "Ecircumflexdotbelow", "Ecircumflexgrave", "Ecircumflexhookabove",
  "Ecircumflexsmall", "Ecircumflextilde", "Ecyrillic", "Edblgrave",
  "Edieresis", "Edieresissmall", "Edot", "Edotaccent", "Edotbelow",
  "Efcyrillic", "Egrave", "Egravesmall", "Eharmenian", "Ehookabove",
  "Eightroman", "Einvertedbreve", "Eiotifiedcyrillic", "Elcyrillic",
  "Elevenroman", "Emacron", "Emacronacute", "Emacrongrave", "Emcyrillic",
  "Emonospace", "Encyrillic", "Endescendercyrillic", "Eng", "Enghecyrillic",
  "Enhookcyrillic", "Eogonek", "Eopen", "Epsilon", "Epsilontonos",
  "Ercyrillic", "Ereversed", "Ereversedcyrillic", "Escyrillic",
  "Esdescendercyrillic", "Esh", "Esmall", "Eta", "Etarmenian", "Etatonos",
  "Eth", "Ethsmall", "Etilde", "Etildebelow", "Euro", "Ezh", "Ezhcaron",
  "Ezhreversed", "F", "Fcircle", "Fdotaccent", "Feharmenian", "Feicoptic",
  "Fhook", "Fitacyrillic", "Fiveroman", "Fmonospace", "Fourroman", "Fsmall",
  "G", "GBsquare", "Gacute", "Gamma", "Gammaafrican", "Gangiacoptic",
  "Gbreve", "Gcaron", "Gcedilla", "Gcircle", "Gcircumflex", "Gcommaaccent",
  "Gdot", "Gdotaccent", "Gecyrillic", "Ghadarmenian", "Ghemiddlehookcyrillic",
  "Ghestrokecyrillic", "Gheupturncyrillic", "Ghook", "Gimarmenian",
  "Gjecyrillic", "Gmacron", "Gmonospace", "Grave", "Gravesmall", "Gsmall",
  "Gsmallhook", "Gstroke", "H", "H18533", "H18543", "H18551", "H22073",
  "HPsquare", "Haabkhasiancyrillic", "Hadescendercyrillic",
  "Hardsigncyrillic", "Hbar", "Hbrevebelow", "Hcedilla", "Hcircle",
  "Hcircumflex", "Hdieresis", "Hdotaccent", "Hdotbelow", "Hmonospace",
  "Hoarmenian", "Horicoptic", "Hsmall", "Hungarumlaut", "Hungarumlautsmall",
  "Hzsquare", "I", "IAcyrillic", "IJ", "IUcyrillic", "Iacute", "Iacutesmall",
  "Ibreve", "Icaron", "Icircle", "Icircumflex", "Icircumflexsmall",
  "Icyrillic", "Idblgrave", "Idieresis", "Idieresisacute",
  "Idieresiscyrillic", "Idieresissmall", "Idot", "Idotaccent", "Idotbelow",
  "Iebrevecyrillic", "Iecyrillic", "Ifraktur", "Igrave", "Igravesmall",
  "Ihookabove", "Iicyrillic", "Iinvertedbreve", "Iishortcyrillic", "Imacron",
  "Imacroncyrillic", "Imonospace", "Iniarmenian", "Iocyrillic", "Iogonek",
  "Iota", "Iotaafrican", "Iotadieresis", "Iotatonos", "Ismall", "Istroke",
  "Itilde", "Itildebelow", "Izhitsacyrillic", "Izhitsadblgravecyrillic", "J",
  "Jaarmenian", "Jcircle", "Jcircumflex", "Jecyrillic", "Jheharmenian",
  "Jmonospace", "Jsmall", "K", "KBsquare", "KKsquare", "Kabashkircyrillic",
  "Kacute", "Kacyrillic", "Kadescendercyrillic", "Kahookcyrillic", "Kappa",
  "Kastrokecyrillic", "Kaverticalstrokecyrillic", "Kcaron", "Kcedilla",
  "Kcircle", "Kcommaaccent", "Kdotbelow", "Keharmenian", "Kenarmenian",
  "Khacyrillic", "Kheicoptic", "Khook", "Kjecyrillic", "Klinebelow",
  "Kmonospace", "Koppacyrillic", "Koppagreek", "Ksicyrillic", "Ksmall", "L",
  "LJ", "LL", "Lacute", "Lambda", "Lcaron", "Lcedilla", "Lcircle",
  "Lcircumflexbelow", "Lcommaaccent", "Ldot", "Ldotaccent", "Ldotbelow",
  "Ldotbelowmacron", "Liwnarmenian", "Lj", "Ljecyrillic", "Llinebelow",
  "Lmonospace", "Lslash", "Lslashsmall", "Lsmall", "M", "MBsquare", "Macron",
  "Macronsmall", "Macute", "Mcircle", "Mdotaccent", "Mdotbelow",
  "Menarmenian", "Mmonospace", "Msmall", "Mturned", "Mu", "N", "NJ", "Nacute",
  "Ncaron", "Ncedilla", "Ncircle", "Ncircumflexbelow", "Ncommaaccent",
  "Ndotaccent", "Ndotbelow", "Nhookleft", "Nineroman", "Nj", "Njecyrillic",
  "Nlinebelow", "Nmonospace", "Nowarmenian", "Nsmall", "Ntilde",
  "Ntildesmall", "Nu", "O", "OE", "OEsmall", "Oacute", "Oacutesmall",
  "Obarredcyrillic", "Obarreddieresiscyrillic", "Obreve", "Ocaron",
  "Ocenteredtilde", "Ocircle", "Ocircumflex", "Ocircumflexacute",
  "Ocircumflexdotbelow", "Ocircumflexgrave", "Ocircumflexhookabove",
  "Ocircumflexsmall", "Ocircumflextilde", "Ocyrillic", "Odblacute",
  "Odblgrave", "Odieresis", "Odieresiscyrillic", "Odieresissmall",
  "Odotbelow", "Ogoneksmall", "Ograve", "Ogravesmall", "Oharmenian", "Ohm",
  "Ohookabove", "Ohorn", "Ohornacute", "Ohorndotbelow", "Ohorngrave",
  "Ohornhookabove", "Ohorntilde", "Ohungarumlaut", "Oi", "Oinvertedbreve",
  "Omacron", "Omacronacute", "Omacrongrave", "Omega", "Omegacyrillic",
  "Omegagreek", "Omegaroundcyrillic", "Omegatitlocyrillic", "Omegatonos",
  "Omicron", "Omicrontonos", "Omonospace", "Oneroman", "Oogonek",
  "Oogonekmacron", "Oopen", "Oslash", "Oslashacute", "Oslashsmall", "Osmall",
  "Ostrokeacute", "Otcyrillic", "Otilde", "Otildeacute", "Otildedieresis",
  "Otildesmall", "P", "Pacute", "Pcircle", "Pdotaccent", "Pecyrillic",
  "Peharmenian", "Pemiddlehookcyrillic", "Phi", "Phook", "Pi", "Piwrarmenian",
  "Pmonospace", "Psi", "Psicyrillic", "Psmall", "Q", "Qcircle", "Qmonospace",
  "Qsmall", "R", "Raarmenian", "Racute", "Rcaron", "Rcedilla", "Rcircle",
  "Rcommaaccent", "Rdblgrave", "Rdotaccent", "Rdotbelow", "Rdotbelowmacron",
  "Reharmenian", "Rfraktur", "Rho", "Ringsmall", "Rinvertedbreve",
  "Rlinebelow", "Rmonospace", "Rsmall", "Rsmallinverted",
  "Rsmallinvertedsuperior", "S", "SF010000", "SF020000", "SF030000",
  "SF040000", "SF050000", "SF060000", "SF070000", "SF080000", "SF090000",
  "SF100000", "SF110000", "SF190000", "SF200000", "SF210000", "SF220000",
  "SF230000", "SF240000", "SF250000", "SF260000", "SF270000", "SF280000",
  "SF360000", "SF370000", "SF380000", "SF390000", "SF400000", "SF410000",
  "SF420000", "SF430000", "SF440000", "SF450000", "SF460000", "SF470000",
  "SF480000", "SF490000", "SF500000", "SF510000", "SF520000", "SF530000",
  "SF540000", "Sacute", "Sacutedotaccent", "Sampigreek", "Scaron",
  "Scarondotaccent", "Scaronsmall", "Scedilla", "Schwa", "Schwacyrillic",
  "Schwadieresiscyrillic", "Scircle", "Scircumflex", "Scommaaccent",
  "Sdotaccent", "Sdotbelow", "Sdotbelowdotaccent", "Seharmenian",
  "Sevenroman", "Shaarmenian", "Shacyrillic", "Shchacyrillic", "Sheicoptic",
  "Shhacyrillic", "Shimacoptic", "Sigma", "Sixroman", "Smonospace",
  "Softsigncyrillic", "Ssmall", "Stigmagreek", "T", "Tau", "Tbar", "Tcaron",
  "Tcedilla", "Tcircle", "Tcircumflexbelow", "Tcommaaccent", "Tdotaccent",
  "Tdotbelow", "Tecyrillic", "Tedescendercyrillic", "Tenroman",
  "Tetsecyrillic", "Theta", "Thook", "Thorn", "Thornsmall", "Threeroman",
  "Tildesmall", "Tiwnarmenian", "Tlinebelow", "Tmonospace", "Toarmenian",
  "Tonefive", "Tonesix", "Tonetwo", "Tretroflexhook", "Tsecyrillic",
  "Tshecyrillic", "Tsmall", "Twelveroman", "Tworoman", "U", "Uacute",
  "Uacutesmall", "Ubreve", "Ucaron", "Ucircle", "Ucircumflex",
  "Ucircumflexbelow", "Ucircumflexsmall", "Ucyrillic", "Udblacute",
  "Udblgrave", "Udieresis", "Udieresisacute", "Udieresisbelow",
  "Udieresiscaron", "Udieresiscyrillic", "Udieresisgrave", "Udieresismacron",
  "Udieresissmall", "Udotbelow", "Ugrave", "Ugravesmall", "Uhookabove",
  "Uhorn", "Uhornacute", "Uhorndotbelow", "Uhorngrave", "Uhornhookabove",
  "Uhorntilde", "Uhungarumlaut", "Uhungarumlautcyrillic", "Uinvertedbreve",
  "Ukcyrillic", "Umacron", "Umacroncyrillic", "Umacrondieresis", "Umonospace",
  "Uogonek", "Upsilon", "Upsilon1", "Upsilonacutehooksymbolgreek",
  "Upsilonafrican", "Upsilondieresis", "Upsilondieresishooksymbolgreek",
  "Upsilonhooksymbol", "Upsilontonos", "Uring", "Ushortcyrillic", "Usmall",
  "Ustraightcyrillic", "Ustraightstrokecyrillic", "Utilde", "Utildeacute",
  "Utildebelow", "V", "Vcircle", "Vdotbelow", "Vecyrillic", "Vewarmenian",
  "Vhook", "Vmonospace", "Voarmenian", "Vsmall", "Vtilde", "W", "Wacute",
  "Wcircle", "Wcircumflex", "Wdieresis", "Wdotaccent", "Wdotbelow", "Wgrave",
  "Wmonospace", "Wsmall", "X", "Xcircle", "Xdieresis", "Xdotaccent",
  "Xeharmenian", "Xi", "Xmonospace", "Xsmall", "Y", "Yacute", "Yacutesmall",
  "Yatcyrillic", "Ycircle", "Ycircumflex", "Ydieresis", "Ydieresissmall",
  "Ydotaccent", "Ydotbelow", "Yericyrillic", "Yerudieresiscyrillic", "Ygrave",
  "Yhook", "Yhookabove", "Yiarmenian", "Yicyrillic", "Yiwnarmenian",
  "Ymonospace", "Ysmall", "Ytilde", "Yusbigcyrillic",
  "Yusbigiotifiedcyrillic", "Yuslittlecyrillic", "Yuslittleiotifiedcyrillic",
  "Z", "Zaarmenian", "Zacute", "Zcaron", "Zcaronsmall", "Zcircle",
  "Zcircumflex", "Zdot", "Zdotaccent", "Zdotbelow", "Zecyrillic",
  "Zedescendercyrillic", "Zedieresiscyrillic", "Zeta", "Zhearmenian",
  "Zhebrevecyrillic", "Zhecyrillic", "Zhedescendercyrillic",
  "Zhedieresiscyrillic", "Zlinebelow", "Zmonospace", "Zsmall", "Zstroke", "a",
  "aabengali", "aacute", "aadeva", "aagujarati", "aagurmukhi",
  "aamatragurmukhi", "aarusquare", "aavowelsignbengali", "aavowelsigndeva",
  "aavowelsigngujarati", "abbreviationmarkarmenian", "abbreviationsigndeva",
  "abengali", "abopomofo", "abreve", "abreveacute", "abrevecyrillic",
  "abrevedotbelow", "abrevegrave", "abrevehookabove", "abrevetilde", "acaron",
  "acircle", "acircumflex", "acircumflexacute", "acircumflexdotbelow",
  "acircumflexgrave", "acircumflexhookabove", "acircumflextilde", "acute",
  "acutebelowcmb", "acutecmb", "acutecomb", "acutedeva", "acutelowmod",
  "acutetonecmb", "acyrillic", "adblgrave", "addakgurmukhi", "adeva",
  "adieresis", "adieresiscyrillic", "adieresismacron", "adotbelow",
  "adotmacron", "ae", "aeacute", "aekorean", "aemacron", "afii00208",
  "afii08941", "afii10017", "afii10018", "afii10019", "afii10020",
  "afii10021", "afii10022", "afii10023", "afii10024", "afii10025",
  "afii10026", "afii10027", "afii10028", "afii10029", "afii10030",
  "afii10031", "afii10032", "afii10033", "afii10034", "afii10035",
  "afii10036", "afii10037", "afii10038", "afii10039", "afii10040",
  "afii10041", "afii10042", "afii10043", "afii10044", "afii10045",
  "afii10046", "afii10047", "afii10048", "afii10049", "afii10050",
  "afii10051", "afii10052", "afii10053", "afii10054", "afii10055",
  "afii10056", "afii10057", "afii10058", "afii10059", "afii10060",
  "afii10061", "afii10062", "afii10063", "afii10064", "afii10065",
  "afii10066", "afii10067", "afii10068", "afii10069", "afii10070",
  "afii10071", "afii10072", "afii10073", "afii10074", "afii10075",
  "afii10076", "afii10077", "afii10078", "afii10079", "afii10080",
  "afii10081", "afii10082", "afii10083", "afii10084", "afii10085",
  "afii10086", "afii10087", "afii10088", "afii10089", "afii10090",
  "afii10091", "afii10092", "afii10093", "afii10094", "afii10095",
  "afii10096", "afii10097", "afii10098", "afii10099", "afii10100",
  "afii10101", "afii10102", "afii10103", "afii10104", "afii10105",
  "afii10106", "afii10107", "afii10108", "afii10109", "afii10110",
  "afii10145", "afii10146", "afii10147", "afii10148", "afii10192",
  "afii10193", "afii10194", "afii10195", "afii10196", "afii10831",
  "afii10832", "afii10846", "afii299", "afii300", "afii301", "afii57381",
  "afii57388", "afii57392", "afii57393", "afii57394", "afii57395",
  "afii57396", "afii57397", "afii57398", "afii57399", "afii57400",
  "afii57401", "afii57403", "afii57407", "afii57409", "afii57410",
  "afii57411", "afii57412", "afii57413", "afii57414", "afii57415",
  "afii57416", "afii57417", "afii57418", "afii57419", "afii57420",
  "afii57421", "afii57422", "afii57423", "afii57424", "afii57425",
  "afii57426", "afii57427", "afii57428", "afii57429", "afii57430",
  "afii57431", "afii57432", "afii57433", "afii57434", "afii57440",
  "afii57441", "afii57442", "afii57443", "afii57444", "afii57445",
  "afii57446", "afii57448", "afii57449", "afii57450", "afii57451",
  "afii57452", "afii57453", "afii57454", "afii57455", "afii57456",
  "afii57457", "afii57458", "afii57470", "afii57505", "afii57506",
  "afii57507", "afii57508", "afii57509", "afii57511", "afii57512",
  "afii57513", "afii57514", "afii57519", "afii57534", "afii57636",
  "afii57645", "afii57658", "afii57664", "afii57665", "afii57666",
  "afii57667", "afii57668", "afii57669", "afii57670", "afii57671",
  "afii57672", "afii57673", "afii57674", "afii57675", "afii57676",
  "afii57677", "afii57678", "afii57679", "afii57680", "afii57681",
  "afii57682", "afii57683", "afii57684", "afii57685", "afii57686",
  "afii57687", "afii57688", "afii57689", "afii57690", "afii57694",
  "afii57695", "afii57700", "afii57705", "afii57716", "afii57717",
  "afii57718", "afii57723", "afii57793", "afii57794", "afii57795",
  "afii57796", "afii57797", "afii57798", "afii57799", "afii57800",
  "afii57801", "afii57802", "afii57803", "afii57804", "afii57806",
  "afii57807", "afii57839", "afii57841", "afii57842", "afii57929",
  "afii61248", "afii61289", "afii61352", "afii61573", "afii61574",
  "afii61575", "afii61664", "afii63167", "afii64937", "agrave", "agujarati",
  "agurmukhi", "ahiragana", "ahookabove", "aibengali", "aibopomofo", "aideva",
  "aiecyrillic", "aigujarati", "aigurmukhi", "aimatragurmukhi", "ainarabic",
  "ainfinalarabic", "aininitialarabic", "ainmedialarabic", "ainvertedbreve",
  "aivowelsignbengali", "aivowelsigndeva", "aivowelsigngujarati", "akatakana",
  "akatakanahalfwidth", "akorean", "alef", "alefarabic", "alefdageshhebrew",
  "aleffinalarabic", "alefhamzaabovearabic", "alefhamzaabovefinalarabic",
  "alefhamzabelowarabic", "alefhamzabelowfinalarabic", "alefhebrew",
  "aleflamedhebrew", "alefmaddaabovearabic", "alefmaddaabovefinalarabic",
  "alefmaksuraarabic", "alefmaksurafinalarabic", "alefmaksurainitialarabic",
  "alefmaksuramedialarabic", "alefpatahhebrew", "alefqamatshebrew", "aleph",
  "allequal", "alpha", "alphatonos", "amacron", "amonospace", "ampersand",
  "ampersandmonospace", "ampersandsmall", "amsquare", "anbopomofo",
  "angbopomofo", "angkhankhuthai", "angle", "anglebracketleft",
  "anglebracketleftvertical", "anglebracketright",
  "anglebracketrightvertical", "angleleft", "angleright", "angstrom",
  "anoteleia", "anudattadeva", "anusvarabengali", "anusvaradeva",
  "anusvaragujarati", "aogonek", "apaatosquare", "aparen",
  "apostrophearmenian", "apostrophemod", "apple", "approaches", "approxequal",
  "approxequalorimage", "approximatelyequal", "araeaekorean", "araeakorean",
  "arc", "arighthalfring", "aring", "aringacute", "aringbelow", "arrowboth",
  "arrowdashdown", "arrowdashleft", "arrowdashright", "arrowdashup",
  "arrowdblboth", "arrowdbldown", "arrowdblleft", "arrowdblright",
  "arrowdblup", "arrowdown", "arrowdownleft", "arrowdownright",
  "arrowdownwhite", "arrowheaddownmod", "arrowheadleftmod",
  "arrowheadrightmod", "arrowheadupmod", "arrowhorizex", "arrowleft",
  "arrowleftdbl", "arrowleftdblstroke", "arrowleftoverright",
  "arrowleftwhite", "arrowright", "arrowrightdblstroke", "arrowrightheavy",
  "arrowrightoverleft", "arrowrightwhite", "arrowtableft", "arrowtabright",
  "arrowup", "arrowupdn", "arrowupdnbse", "arrowupdownbase", "arrowupleft",
  "arrowupleftofdown", "arrowupright", "arrowupwhite", "arrowvertex",
  "asciicircum", "asciicircummonospace", "asciitilde", "asciitildemonospace",
  "ascript", "ascriptturned", "asmallhiragana", "asmallkatakana",
  "asmallkatakanahalfwidth", "asterisk", "asteriskaltonearabic",
  "asteriskarabic", "asteriskmath", "asteriskmonospace", "asterisksmall",
  "asterism", "asuperior", "asymptoticallyequal", "at", "atilde",
  "atmonospace", "atsmall", "aturned", "aubengali", "aubopomofo", "audeva",
  "augujarati", "augurmukhi", "aulengthmarkbengali", "aumatragurmukhi",
  "auvowelsignbengali", "auvowelsigndeva", "auvowelsigngujarati",
  "avagrahadeva", "aybarmenian", "ayin", "ayinaltonehebrew", "ayinhebrew",
  "b", "babengali", "backslash", "backslashmonospace", "badeva", "bagujarati",
  "bagurmukhi", "bahiragana", "bahtthai", "bakatakana", "bar", "barmonospace",
  "bbopomofo", "bcircle", "bdotaccent", "bdotbelow", "beamedsixteenthnotes",
  "because", "becyrillic", "beharabic", "behfinalarabic", "behinitialarabic",
  "behiragana", "behmedialarabic", "behmeeminitialarabic",
  "behmeemisolatedarabic", "behnoonfinalarabic", "bekatakana", "benarmenian",
  "bet", "beta", "betasymbolgreek", "betdagesh", "betdageshhebrew",
  "bethebrew", "betrafehebrew", "bhabengali", "bhadeva", "bhagujarati",
  "bhagurmukhi", "bhook", "bihiragana", "bikatakana", "bilabialclick",
  "bindigurmukhi", "birusquare", "blackcircle", "blackdiamond",
  "blackdownpointingtriangle", "blackleftpointingpointer",
  "blackleftpointingtriangle", "blacklenticularbracketleft",
  "blacklenticularbracketleftvertical", "blacklenticularbracketright",
  "blacklenticularbracketrightvertical", "blacklowerlefttriangle",
  "blacklowerrighttriangle", "blackrectangle", "blackrightpointingpointer",
  "blackrightpointingtriangle", "blacksmallsquare", "blacksmilingface",
  "blacksquare", "blackstar", "blackupperlefttriangle",
  "blackupperrighttriangle", "blackuppointingsmalltriangle",
  "blackuppointingtriangle", "blank", "blinebelow", "block", "bmonospace",
  "bobaimaithai", "bohiragana", "bokatakana", "bparen", "bqsquare", "braceex",
  "braceleft", "braceleftbt", "braceleftmid", "braceleftmonospace",
  "braceleftsmall", "bracelefttp", "braceleftvertical", "braceright",
  "bracerightbt", "bracerightmid", "bracerightmonospace", "bracerightsmall",
  "bracerighttp", "bracerightvertical", "bracketleft", "bracketleftbt",
  "bracketleftex", "bracketleftmonospace", "bracketlefttp", "bracketright",
  "bracketrightbt", "bracketrightex", "bracketrightmonospace",
  "bracketrighttp", "breve", "brevebelowcmb", "brevecmb",
  "breveinvertedbelowcmb", "breveinvertedcmb", "breveinverteddoublecmb",
  "bridgebelowcmb", "bridgeinvertedbelowcmb", "brokenbar", "bstroke",
  "bsuperior", "btopbar", "buhiragana", "bukatakana", "bullet",
  "bulletinverse", "bulletoperator", "bullseye", "c", "caarmenian",
  "cabengali", "cacute", "cadeva", "cagujarati", "cagurmukhi", "calsquare",
  "candrabindubengali", "candrabinducmb", "candrabindudeva",
  "candrabindugujarati", "capslock", "careof", "caron", "caronbelowcmb",
  "caroncmb", "carriagereturn", "cbopomofo", "ccaron", "ccedilla",
  "ccedillaacute", "ccircle", "ccircumflex", "ccurl", "cdot", "cdotaccent",
  "cdsquare", "cedilla", "cedillacmb", "cent", "centigrade", "centinferior",
  "centmonospace", "centoldstyle", "centsuperior", "chaarmenian",
  "chabengali", "chadeva", "chagujarati", "chagurmukhi", "chbopomofo",
  "cheabkhasiancyrillic", "checkmark", "checyrillic",
  "chedescenderabkhasiancyrillic", "chedescendercyrillic",
  "chedieresiscyrillic", "cheharmenian", "chekhakassiancyrillic",
  "cheverticalstrokecyrillic", "chi", "chieuchacirclekorean",
  "chieuchaparenkorean", "chieuchcirclekorean", "chieuchkorean",
  "chieuchparenkorean", "chochangthai", "chochanthai", "chochingthai",
  "chochoethai", "chook", "cieucacirclekorean", "cieucaparenkorean",
  "cieuccirclekorean", "cieuckorean", "cieucparenkorean", "cieucuparenkorean",
  "circle", "circlemultiply", "circleot", "circleplus", "circlepostalmark",
  "circlewithlefthalfblack", "circlewithrighthalfblack", "circumflex",
  "circumflexbelowcmb", "circumflexcmb", "clear", "clickalveolar",
  "clickdental", "clicklateral", "clickretroflex", "club", "clubsuitblack",
  "clubsuitwhite", "cmcubedsquare", "cmonospace", "cmsquaredsquare",
  "coarmenian", "colon", "colonmonetary", "colonmonospace", "colonsign",
  "colonsmall", "colontriangularhalfmod", "colontriangularmod", "comma",
  "commaabovecmb", "commaaboverightcmb", "commaaccent", "commaarabic",
  "commaarmenian", "commainferior", "commamonospace", "commareversedabovecmb",
  "commareversedmod", "commasmall", "commasuperior", "commaturnedabovecmb",
  "commaturnedmod", "compass", "congruent", "contourintegral", "control",
  "controlACK", "controlBEL", "controlBS", "controlCAN", "controlCR",
  "controlDC1", "controlDC2", "controlDC3", "controlDC4", "controlDEL",
  "controlDLE", "controlEM", "controlENQ", "controlEOT", "controlESC",
  "controlETB", "controlETX", "controlFF", "controlFS", "controlGS",
  "controlHT", "controlLF", "controlNAK", "controlRS", "controlSI",
  "controlSO", "controlSOT", "controlSTX", "controlSUB", "controlSYN",
  "controlUS", "controlVT", "copyright", "copyrightsans", "copyrightserif",
  "cornerbracketleft", "cornerbracketlefthalfwidth",
  "cornerbracketleftvertical", "cornerbracketright",
  "cornerbracketrighthalfwidth", "cornerbracketrightvertical",
  "corporationsquare", "cosquare", "coverkgsquare", "cparen", "cruzeiro",
  "cstretched", "curlyand", "curlyor", "currency", "cyrBreve", "cyrFlex",
  "cyrbreve", "cyrflex", "d", "daarmenian", "dabengali", "dadarabic",
  "dadeva", "dadfinalarabic", "dadinitialarabic", "dadmedialarabic", "dagesh",
  "dageshhebrew", "dagger", "daggerdbl", "dagujarati", "dagurmukhi",
  "dahiragana", "dakatakana", "dalarabic", "dalet", "daletdagesh",
  "daletdageshhebrew", "dalethatafpatah", "dalethatafpatahhebrew",
  "dalethatafsegol", "dalethatafsegolhebrew", "dalethebrew", "dalethiriq",
  "dalethiriqhebrew", "daletholam", "daletholamhebrew", "daletpatah",
  "daletpatahhebrew", "daletqamats", "daletqamatshebrew", "daletqubuts",
  "daletqubutshebrew", "daletsegol", "daletsegolhebrew", "daletsheva",
  "daletshevahebrew", "dalettsere", "dalettserehebrew", "dalfinalarabic",
  "dammaarabic", "dammalowarabic", "dammatanaltonearabic", "dammatanarabic",
  "danda", "dargahebrew", "dargalefthebrew", "dasiapneumatacyrilliccmb",
  "dblGrave", "dblanglebracketleft", "dblanglebracketleftvertical",
  "dblanglebracketright", "dblanglebracketrightvertical",
  "dblarchinvertedbelowcmb", "dblarrowleft", "dblarrowright", "dbldanda",
  "dblgrave", "dblgravecmb", "dblintegral", "dbllowline", "dbllowlinecmb",
  "dbloverlinecmb", "dblprimemod", "dblverticalbar",
  "dblverticallineabovecmb", "dbopomofo", "dbsquare", "dcaron", "dcedilla",
  "dcircle", "dcircumflexbelow", "dcroat", "ddabengali", "ddadeva",
  "ddagujarati", "ddagurmukhi", "ddalarabic", "ddalfinalarabic", "dddhadeva",
  "ddhabengali", "ddhadeva", "ddhagujarati", "ddhagurmukhi", "ddotaccent",
  "ddotbelow", "decimalseparatorarabic", "decimalseparatorpersian",
  "decyrillic", "degree", "dehihebrew", "dehiragana", "deicoptic",
  "dekatakana", "deleteleft", "deleteright", "delta", "deltaturned",
  "denominatorminusonenumeratorbengali", "dezh", "dhabengali", "dhadeva",
  "dhagujarati", "dhagurmukhi", "dhook", "dialytikatonos",
  "dialytikatonoscmb", "diamond", "diamondsuitwhite", "dieresis",
  "dieresisacute", "dieresisbelowcmb", "dieresiscmb", "dieresisgrave",
  "dieresistonos", "dihiragana", "dikatakana", "dittomark", "divide",
  "divides", "divisionslash", "djecyrillic", "dkshade", "dlinebelow",
  "dlsquare", "dmacron", "dmonospace", "dnblock", "dochadathai", "dodekthai",
  "dohiragana", "dokatakana", "dollar", "dollarinferior", "dollarmonospace",
  "dollaroldstyle", "dollarsmall", "dollarsuperior", "dong", "dorusquare",
  "dotaccent", "dotaccentcmb", "dotbelowcmb", "dotbelowcomb", "dotkatakana",
  "dotlessi", "dotlessj", "dotlessjstrokehook", "dotmath", "dottedcircle",
  "doubleyodpatah", "doubleyodpatahhebrew", "downtackbelowcmb", "downtackmod",
  "dparen", "dsuperior", "dtail", "dtopbar", "duhiragana", "dukatakana", "dz",
  "dzaltone", "dzcaron", "dzcurl", "dzeabkhasiancyrillic", "dzecyrillic",
  "dzhecyrillic", "e", "eacute", "earth", "ebengali", "ebopomofo", "ebreve",
  "ecandradeva", "ecandragujarati", "ecandravowelsigndeva",
  "ecandravowelsigngujarati", "ecaron", "ecedillabreve", "echarmenian",
  "echyiwnarmenian", "ecircle", "ecircumflex", "ecircumflexacute",
  "ecircumflexbelow", "ecircumflexdotbelow", "ecircumflexgrave",
  "ecircumflexhookabove", "ecircumflextilde", "ecyrillic", "edblgrave",
  "edeva", "edieresis", "edot", "edotaccent", "edotbelow", "eegurmukhi",
  "eematragurmukhi", "efcyrillic", "egrave", "egujarati", "eharmenian",
  "ehbopomofo", "ehiragana", "ehookabove", "eibopomofo", "eight",
  "eightarabic", "eightbengali", "eightcircle", "eightcircleinversesansserif",
  "eightdeva", "eighteencircle", "eighteenparen", "eighteenperiod",
  "eightgujarati", "eightgurmukhi", "eighthackarabic", "eighthangzhou",
  "eighthnotebeamed", "eightideographicparen", "eightinferior",
  "eightmonospace", "eightoldstyle", "eightparen", "eightperiod",
  "eightpersian", "eightroman", "eightsuperior", "eightthai",
  "einvertedbreve", "eiotifiedcyrillic", "ekatakana", "ekatakanahalfwidth",
  "ekonkargurmukhi", "ekorean", "elcyrillic", "element", "elevencircle",
  "elevenparen", "elevenperiod", "elevenroman", "ellipsis",
  "ellipsisvertical", "emacron", "emacronacute", "emacrongrave", "emcyrillic",
  "emdash", "emdashvertical", "emonospace", "emphasismarkarmenian",
  "emptyset", "enbopomofo", "encyrillic", "endash", "endashvertical",
  "endescendercyrillic", "eng", "engbopomofo", "enghecyrillic",
  "enhookcyrillic", "enspace", "eogonek", "eokorean", "eopen", "eopenclosed",
  "eopenreversed", "eopenreversedclosed", "eopenreversedhook", "eparen",
  "epsilon", "epsilontonos", "equal", "equalmonospace", "equalsmall",
  "equalsuperior", "equivalence", "erbopomofo", "ercyrillic", "ereversed",
  "ereversedcyrillic", "escyrillic", "esdescendercyrillic", "esh", "eshcurl",
  "eshortdeva", "eshortvowelsigndeva", "eshreversedloop", "eshsquatreversed",
  "esmallhiragana", "esmallkatakana", "esmallkatakanahalfwidth", "estimated",
  "esuperior", "eta", "etarmenian", "etatonos", "eth", "etilde",
  "etildebelow", "etnahtafoukhhebrew", "etnahtafoukhlefthebrew",
  "etnahtahebrew", "etnahtalefthebrew", "eturned", "eukorean", "euro",
  "evowelsignbengali", "evowelsigndeva", "evowelsigngujarati", "exclam",
  "exclamarmenian", "exclamdbl", "exclamdown", "exclamdownsmall",
  "exclammonospace", "exclamsmall", "existential", "ezh", "ezhcaron",
  "ezhcurl", "ezhreversed", "ezhtail", "f", "fadeva", "fagurmukhi",
  "fahrenheit", "fathaarabic", "fathalowarabic", "fathatanarabic",
  "fbopomofo", "fcircle", "fdotaccent", "feharabic", "feharmenian",
  "fehfinalarabic", "fehinitialarabic", "fehmedialarabic", "feicoptic",
  "female", "ff", "ffi", "ffl", "fi", "fifteencircle", "fifteenparen",
  "fifteenperiod", "figuredash", "filledbox", "filledrect", "finalkaf",
  "finalkafdagesh", "finalkafdageshhebrew", "finalkafhebrew",
  "finalkafqamats", "finalkafqamatshebrew", "finalkafsheva",
  "finalkafshevahebrew", "finalmem", "finalmemhebrew", "finalnun",
  "finalnunhebrew", "finalpe", "finalpehebrew", "finaltsadi",
  "finaltsadihebrew", "firsttonechinese", "fisheye", "fitacyrillic", "five",
  "fivearabic", "fivebengali", "fivecircle", "fivecircleinversesansserif",
  "fivedeva", "fiveeighths", "fivegujarati", "fivegurmukhi", "fivehackarabic",
  "fivehangzhou", "fiveideographicparen", "fiveinferior", "fivemonospace",
  "fiveoldstyle", "fiveparen", "fiveperiod", "fivepersian", "fiveroman",
  "fivesuperior", "fivethai", "fl", "florin", "fmonospace", "fmsquare",
  "fofanthai", "fofathai", "fongmanthai", "forall", "four", "fourarabic",
  "fourbengali", "fourcircle", "fourcircleinversesansserif", "fourdeva",
  "fourgujarati", "fourgurmukhi", "fourhackarabic", "fourhangzhou",
  "fourideographicparen", "fourinferior", "fourmonospace",
  "fournumeratorbengali", "fouroldstyle", "fourparen", "fourperiod",
  "fourpersian", "fourroman", "foursuperior", "fourteencircle",
  "fourteenparen", "fourteenperiod", "fourthai", "fourthtonechinese",
  "fparen", "fraction", "franc", "g", "gabengali", "gacute", "gadeva",
  "gafarabic", "gaffinalarabic", "gafinitialarabic", "gafmedialarabic",
  "gagujarati", "gagurmukhi", "gahiragana", "gakatakana", "gamma",
  "gammalatinsmall", "gammasuperior", "gangiacoptic", "gbopomofo", "gbreve",
  "gcaron", "gcedilla", "gcircle", "gcircumflex", "gcommaaccent", "gdot",
  "gdotaccent", "gecyrillic", "gehiragana", "gekatakana",
  "geometricallyequal", "gereshaccenthebrew", "gereshhebrew",
  "gereshmuqdamhebrew", "germandbls", "gershayimaccenthebrew",
  "gershayimhebrew", "getamark", "ghabengali", "ghadarmenian", "ghadeva",
  "ghagujarati", "ghagurmukhi", "ghainarabic", "ghainfinalarabic",
  "ghaininitialarabic", "ghainmedialarabic", "ghemiddlehookcyrillic",
  "ghestrokecyrillic", "gheupturncyrillic", "ghhadeva", "ghhagurmukhi",
  "ghook", "ghzsquare", "gihiragana", "gikatakana", "gimarmenian", "gimel",
  "gimeldagesh", "gimeldageshhebrew", "gimelhebrew", "gjecyrillic",
  "glottalinvertedstroke", "glottalstop", "glottalstopinverted",
  "glottalstopmod", "glottalstopreversed", "glottalstopreversedmod",
  "glottalstopreversedsuperior", "glottalstopstroke",
  "glottalstopstrokereversed", "gmacron", "gmonospace", "gohiragana",
  "gokatakana", "gparen", "gpasquare", "gradient", "grave", "gravebelowcmb",
  "gravecmb", "gravecomb", "gravedeva", "gravelowmod", "gravemonospace",
  "gravetonecmb", "greater", "greaterequal", "greaterequalorless",
  "greatermonospace", "greaterorequivalent", "greaterorless",
  "greateroverequal", "greatersmall", "gscript", "gstroke", "guhiragana",
  "guillemotleft", "guillemotright", "guilsinglleft", "guilsinglright",
  "gukatakana", "guramusquare", "gysquare", "h", "haabkhasiancyrillic",
  "haaltonearabic", "habengali", "hadescendercyrillic", "hadeva",
  "hagujarati", "hagurmukhi", "haharabic", "hahfinalarabic",
  "hahinitialarabic", "hahiragana", "hahmedialarabic", "haitusquare",
  "hakatakana", "hakatakanahalfwidth", "halantgurmukhi", "hamzaarabic",
  "hamzadammaarabic", "hamzadammatanarabic", "hamzafathaarabic",
  "hamzafathatanarabic", "hamzalowarabic", "hamzalowkasraarabic",
  "hamzalowkasratanarabic", "hamzasukunarabic", "hangulfiller",
  "hardsigncyrillic", "harpoonleftbarbup", "harpoonrightbarbup", "hasquare",
  "hatafpatah", "hatafpatah16", "hatafpatah23", "hatafpatah2f",
  "hatafpatahhebrew", "hatafpatahnarrowhebrew", "hatafpatahquarterhebrew",
  "hatafpatahwidehebrew", "hatafqamats", "hatafqamats1b", "hatafqamats28",
  "hatafqamats34", "hatafqamatshebrew", "hatafqamatsnarrowhebrew",
  "hatafqamatsquarterhebrew", "hatafqamatswidehebrew", "hatafsegol",
  "hatafsegol17", "hatafsegol24", "hatafsegol30", "hatafsegolhebrew",
  "hatafsegolnarrowhebrew", "hatafsegolquarterhebrew", "hatafsegolwidehebrew",
  "hbar", "hbopomofo", "hbrevebelow", "hcedilla", "hcircle", "hcircumflex",
  "hdieresis", "hdotaccent", "hdotbelow", "he", "heart", "heartsuitblack",
  "heartsuitwhite", "hedagesh", "hedageshhebrew", "hehaltonearabic",
  "heharabic", "hehebrew", "hehfinalaltonearabic", "hehfinalalttwoarabic",
  "hehfinalarabic", "hehhamzaabovefinalarabic", "hehhamzaaboveisolatedarabic",
  "hehinitialaltonearabic", "hehinitialarabic", "hehiragana",
  "hehmedialaltonearabic", "hehmedialarabic", "heiseierasquare", "hekatakana",
  "hekatakanahalfwidth", "hekutaarusquare", "henghook", "herutusquare", "het",
  "hethebrew", "hhook", "hhooksuperior", "hieuhacirclekorean",
  "hieuhaparenkorean", "hieuhcirclekorean", "hieuhkorean", "hieuhparenkorean",
  "hihiragana", "hikatakana", "hikatakanahalfwidth", "hiriq", "hiriq14",
  "hiriq21", "hiriq2d", "hiriqhebrew", "hiriqnarrowhebrew",
  "hiriqquarterhebrew", "hiriqwidehebrew", "hlinebelow", "hmonospace",
  "hoarmenian", "hohipthai", "hohiragana", "hokatakana",
  "hokatakanahalfwidth", "holam", "holam19", "holam26", "holam32",
  "holamhebrew", "holamnarrowhebrew", "holamquarterhebrew", "holamwidehebrew",
  "honokhukthai", "hookabovecomb", "hookcmb", "hookpalatalizedbelowcmb",
  "hookretroflexbelowcmb", "hoonsquare", "horicoptic", "horizontalbar",
  "horncmb", "hotsprings", "house", "hparen", "hsuperior", "hturned",
  "huhiragana", "huiitosquare", "hukatakana", "hukatakanahalfwidth",
  "hungarumlaut", "hungarumlautcmb", "hv", "hyphen", "hypheninferior",
  "hyphenmonospace", "hyphensmall", "hyphensuperior", "hyphentwo", "i",
  "iacute", "iacyrillic", "ibengali", "ibopomofo", "ibreve", "icaron",
  "icircle", "icircumflex", "icyrillic", "idblgrave", "ideographearthcircle",
  "ideographfirecircle", "ideographicallianceparen", "ideographiccallparen",
  "ideographiccentrecircle", "ideographicclose", "ideographiccomma",
  "ideographiccommaleft", "ideographiccongratulationparen",
  "ideographiccorrectcircle", "ideographicearthparen",
  "ideographicenterpriseparen", "ideographicexcellentcircle",
  "ideographicfestivalparen", "ideographicfinancialcircle",
  "ideographicfinancialparen", "ideographicfireparen", "ideographichaveparen",
  "ideographichighcircle", "ideographiciterationmark",
  "ideographiclaborcircle", "ideographiclaborparen", "ideographicleftcircle",
  "ideographiclowcircle", "ideographicmedicinecircle",
  "ideographicmetalparen", "ideographicmoonparen", "ideographicnameparen",
  "ideographicperiod", "ideographicprintcircle", "ideographicreachparen",
  "ideographicrepresentparen", "ideographicresourceparen",
  "ideographicrightcircle", "ideographicsecretcircle", "ideographicselfparen",
  "ideographicsocietyparen", "ideographicspace", "ideographicspecialparen",
  "ideographicstockparen", "ideographicstudyparen", "ideographicsunparen",
  "ideographicsuperviseparen", "ideographicwaterparen",
  "ideographicwoodparen", "ideographiczero", "ideographmetalcircle",
  "ideographmooncircle", "ideographnamecircle", "ideographsuncircle",
  "ideographwatercircle", "ideographwoodcircle", "ideva", "idieresis",
  "idieresisacute", "idieresiscyrillic", "idotbelow", "iebrevecyrillic",
  "iecyrillic", "ieungacirclekorean", "ieungaparenkorean",
  "ieungcirclekorean", "ieungkorean", "ieungparenkorean", "igrave",
  "igujarati", "igurmukhi", "ihiragana", "ihookabove", "iibengali",
  "iicyrillic", "iideva", "iigujarati", "iigurmukhi", "iimatragurmukhi",
  "iinvertedbreve", "iishortcyrillic", "iivowelsignbengali",
  "iivowelsigndeva", "iivowelsigngujarati", "ij", "ikatakana",
  "ikatakanahalfwidth", "ikorean", "ilde", "iluyhebrew", "imacron",
  "imacroncyrillic", "imageorapproximatelyequal", "imatragurmukhi",
  "imonospace", "increment", "infinity", "iniarmenian", "integral",
  "integralbottom", "integralbt", "integralex", "integraltop", "integraltp",
  "intersection", "intisquare", "invbullet", "invcircle", "invsmileface",
  "iocyrillic", "iogonek", "iota", "iotadieresis", "iotadieresistonos",
  "iotalatin", "iotatonos", "iparen", "irigurmukhi", "ismallhiragana",
  "ismallkatakana", "ismallkatakanahalfwidth", "issharbengali", "istroke",
  "isuperior", "iterationhiragana", "iterationkatakana", "itilde",
  "itildebelow", "iubopomofo", "iucyrillic", "ivowelsignbengali",
  "ivowelsigndeva", "ivowelsigngujarati", "izhitsacyrillic",
  "izhitsadblgravecyrillic", "j", "jaarmenian", "jabengali", "jadeva",
  "jagujarati", "jagurmukhi", "jbopomofo", "jcaron", "jcircle", "jcircumflex",
  "jcrossedtail", "jdotlessstroke", "jecyrillic", "jeemarabic",
  "jeemfinalarabic", "jeeminitialarabic", "jeemmedialarabic", "jeharabic",
  "jehfinalarabic", "jhabengali", "jhadeva", "jhagujarati", "jhagurmukhi",
  "jheharmenian", "jis", "jmonospace", "jparen", "jsuperior", "k",
  "kabashkircyrillic", "kabengali", "kacute", "kacyrillic",
  "kadescendercyrillic", "kadeva", "kaf", "kafarabic", "kafdagesh",
  "kafdageshhebrew", "kaffinalarabic", "kafhebrew", "kafinitialarabic",
  "kafmedialarabic", "kafrafehebrew", "kagujarati", "kagurmukhi",
  "kahiragana", "kahookcyrillic", "kakatakana", "kakatakanahalfwidth",
  "kappa", "kappasymbolgreek", "kapyeounmieumkorean", "kapyeounphieuphkorean",
  "kapyeounpieupkorean", "kapyeounssangpieupkorean", "karoriisquare",
  "kashidaautoarabic", "kashidaautonosidebearingarabic", "kasmallkatakana",
  "kasquare", "kasraarabic", "kasratanarabic", "kastrokecyrillic",
  "katahiraprolongmarkhalfwidth", "kaverticalstrokecyrillic", "kbopomofo",
  "kcalsquare", "kcaron", "kcedilla", "kcircle", "kcommaaccent", "kdotbelow",
  "keharmenian", "kehiragana", "kekatakana", "kekatakanahalfwidth",
  "kenarmenian", "kesmallkatakana", "kgreenlandic", "khabengali",
  "khacyrillic", "khadeva", "khagujarati", "khagurmukhi", "khaharabic",
  "khahfinalarabic", "khahinitialarabic", "khahmedialarabic", "kheicoptic",
  "khhadeva", "khhagurmukhi", "khieukhacirclekorean", "khieukhaparenkorean",
  "khieukhcirclekorean", "khieukhkorean", "khieukhparenkorean", "khokhaithai",
  "khokhonthai", "khokhuatthai", "khokhwaithai", "khomutthai", "khook",
  "khorakhangthai", "khzsquare", "kihiragana", "kikatakana",
  "kikatakanahalfwidth", "kiroguramusquare", "kiromeetorusquare",
  "kirosquare", "kiyeokacirclekorean", "kiyeokaparenkorean",
  "kiyeokcirclekorean", "kiyeokkorean", "kiyeokparenkorean",
  "kiyeoksioskorean", "kjecyrillic", "klinebelow", "klsquare",
  "kmcubedsquare", "kmonospace", "kmsquaredsquare", "kohiragana",
  "kohmsquare", "kokaithai", "kokatakana", "kokatakanahalfwidth",
  "kooposquare", "koppacyrillic", "koreanstandardsymbol", "koroniscmb",
  "kparen", "kpasquare", "ksicyrillic", "ktsquare", "kturned", "kuhiragana",
  "kukatakana", "kukatakanahalfwidth", "kvsquare", "kwsquare", "l",
  "labengali", "lacute", "ladeva", "lagujarati", "lagurmukhi",
  "lakkhangyaothai", "lamaleffinalarabic", "lamalefhamzaabovefinalarabic",
  "lamalefhamzaaboveisolatedarabic", "lamalefhamzabelowfinalarabic",
  "lamalefhamzabelowisolatedarabic", "lamalefisolatedarabic",
  "lamalefmaddaabovefinalarabic", "lamalefmaddaaboveisolatedarabic",
  "lamarabic", "lambda", "lambdastroke", "lamed", "lameddagesh",
  "lameddageshhebrew", "lamedhebrew", "lamedholam", "lamedholamdagesh",
  "lamedholamdageshhebrew", "lamedholamhebrew", "lamfinalarabic",
  "lamhahinitialarabic", "laminitialarabic", "lamjeeminitialarabic",
  "lamkhahinitialarabic", "lamlamhehisolatedarabic", "lammedialarabic",
  "lammeemhahinitialarabic", "lammeeminitialarabic",
  "lammeemjeeminitialarabic", "lammeemkhahinitialarabic", "largecircle",
  "lbar", "lbelt", "lbopomofo", "lcaron", "lcedilla", "lcircle",
  "lcircumflexbelow", "lcommaaccent", "ldot", "ldotaccent", "ldotbelow",
  "ldotbelowmacron", "leftangleabovecmb", "lefttackbelowcmb", "less",
  "lessequal", "lessequalorgreater", "lessmonospace", "lessorequivalent",
  "lessorgreater", "lessoverequal", "lesssmall", "lezh", "lfblock",
  "lhookretroflex", "lira", "liwnarmenian", "lj", "ljecyrillic", "ll",
  "lladeva", "llagujarati", "llinebelow", "llladeva", "llvocalicbengali",
  "llvocalicdeva", "llvocalicvowelsignbengali", "llvocalicvowelsigndeva",
  "lmiddletilde", "lmonospace", "lmsquare", "lochulathai", "logicaland",
  "logicalnot", "logicalnotreversed", "logicalor", "lolingthai", "longs",
  "lowlinecenterline", "lowlinecmb", "lowlinedashed", "lozenge", "lparen",
  "lslash", "lsquare", "lsuperior", "ltshade", "luthai", "lvocalicbengali",
  "lvocalicdeva", "lvocalicvowelsignbengali", "lvocalicvowelsigndeva",
  "lxsquare", "m", "mabengali", "macron", "macronbelowcmb", "macroncmb",
  "macronlowmod", "macronmonospace", "macute", "madeva", "magujarati",
  "magurmukhi", "mahapakhhebrew", "mahapakhlefthebrew", "mahiragana",
  "maichattawalowleftthai", "maichattawalowrightthai", "maichattawathai",
  "maichattawaupperleftthai", "maieklowleftthai", "maieklowrightthai",
  "maiekthai", "maiekupperleftthai", "maihanakatleftthai", "maihanakatthai",
  "maitaikhuleftthai", "maitaikhuthai", "maitholowleftthai",
  "maitholowrightthai", "maithothai", "maithoupperleftthai",
  "maitrilowleftthai", "maitrilowrightthai", "maitrithai",
  "maitriupperleftthai", "maiyamokthai", "makatakana", "makatakanahalfwidth",
  "male", "mansyonsquare", "maqafhebrew", "mars", "masoracirclehebrew",
  "masquare", "mbopomofo", "mbsquare", "mcircle", "mcubedsquare",
  "mdotaccent", "mdotbelow", "meemarabic", "meemfinalarabic",
  "meeminitialarabic", "meemmedialarabic", "meemmeeminitialarabic",
  "meemmeemisolatedarabic", "meetorusquare", "mehiragana", "meizierasquare",
  "mekatakana", "mekatakanahalfwidth", "mem", "memdagesh", "memdageshhebrew",
  "memhebrew", "menarmenian", "merkhahebrew", "merkhakefulahebrew",
  "merkhakefulalefthebrew", "merkhalefthebrew", "mhook", "mhzsquare",
  "middledotkatakanahalfwidth", "middot", "mieumacirclekorean",
  "mieumaparenkorean", "mieumcirclekorean", "mieumkorean",
  "mieumpansioskorean", "mieumparenkorean", "mieumpieupkorean",
  "mieumsioskorean", "mihiragana", "mikatakana", "mikatakanahalfwidth",
  "minus", "minusbelowcmb", "minuscircle", "minusmod", "minusplus", "minute",
  "miribaarusquare", "mirisquare", "mlonglegturned", "mlsquare",
  "mmcubedsquare", "mmonospace", "mmsquaredsquare", "mohiragana",
  "mohmsquare", "mokatakana", "mokatakanahalfwidth", "molsquare", "momathai",
  "moverssquare", "moverssquaredsquare", "mparen", "mpasquare", "mssquare",
  "msuperior", "mturned", "mu", "mu1", "muasquare", "muchgreater", "muchless",
  "mufsquare", "mugreek", "mugsquare", "muhiragana", "mukatakana",
  "mukatakanahalfwidth", "mulsquare", "multiply", "mumsquare", "munahhebrew",
  "munahlefthebrew", "musicalnote", "musicalnotedbl", "musicflatsign",
  "musicsharpsign", "mussquare", "muvsquare", "muwsquare", "mvmegasquare",
  "mvsquare", "mwmegasquare", "mwsquare", "n", "nabengali", "nabla", "nacute",
  "nadeva", "nagujarati", "nagurmukhi", "nahiragana", "nakatakana",
  "nakatakanahalfwidth", "napostrophe", "nasquare", "nbopomofo", "nbspace",
  "ncaron", "ncedilla", "ncircle", "ncircumflexbelow", "ncommaaccent",
  "ndotaccent", "ndotbelow", "nehiragana", "nekatakana",
  "nekatakanahalfwidth", "newsheqelsign", "nfsquare", "ngabengali", "ngadeva",
  "ngagujarati", "ngagurmukhi", "ngonguthai", "nhiragana", "nhookleft",
  "nhookretroflex", "nieunacirclekorean", "nieunaparenkorean",
  "nieuncieuckorean", "nieuncirclekorean", "nieunhieuhkorean", "nieunkorean",
  "nieunpansioskorean", "nieunparenkorean", "nieunsioskorean",
  "nieuntikeutkorean", "nihiragana", "nikatakana", "nikatakanahalfwidth",
  "nikhahitleftthai", "nikhahitthai", "nine", "ninearabic", "ninebengali",
  "ninecircle", "ninecircleinversesansserif", "ninedeva", "ninegujarati",
  "ninegurmukhi", "ninehackarabic", "ninehangzhou", "nineideographicparen",
  "nineinferior", "ninemonospace", "nineoldstyle", "nineparen", "nineperiod",
  "ninepersian", "nineroman", "ninesuperior", "nineteencircle",
  "nineteenparen", "nineteenperiod", "ninethai", "nj", "njecyrillic",
  "nkatakana", "nkatakanahalfwidth", "nlegrightlong", "nlinebelow",
  "nmonospace", "nmsquare", "nnabengali", "nnadeva", "nnagujarati",
  "nnagurmukhi", "nnnadeva", "nohiragana", "nokatakana",
  "nokatakanahalfwidth", "nonbreakingspace", "nonenthai", "nonuthai",
  "noonarabic", "noonfinalarabic", "noonghunnaarabic",
  "noonghunnafinalarabic", "noonhehinitialarabic", "nooninitialarabic",
  "noonjeeminitialarabic", "noonjeemisolatedarabic", "noonmedialarabic",
  "noonmeeminitialarabic", "noonmeemisolatedarabic", "noonnoonfinalarabic",
  "notcontains", "notelement", "notelementof", "notequal", "notgreater",
  "notgreaternorequal", "notgreaternorless", "notidentical", "notless",
  "notlessnorequal", "notparallel", "notprecedes", "notsubset", "notsucceeds",
  "notsuperset", "nowarmenian", "nparen", "nssquare", "nsuperior", "ntilde",
  "nu", "nuhiragana", "nukatakana", "nukatakanahalfwidth", "nuktabengali",
  "nuktadeva", "nuktagujarati", "nuktagurmukhi", "numbersign",
  "numbersignmonospace", "numbersignsmall", "numeralsigngreek",
  "numeralsignlowergreek", "numero", "nun", "nundagesh", "nundageshhebrew",
  "nunhebrew", "nvsquare", "nwsquare", "nyabengali", "nyadeva", "nyagujarati",
  "nyagurmukhi", "o", "oacute", "oangthai", "obarred", "obarredcyrillic",
  "obarreddieresiscyrillic", "obengali", "obopomofo", "obreve", "ocandradeva",
  "ocandragujarati", "ocandravowelsigndeva", "ocandravowelsigngujarati",
  "ocaron", "ocircle", "ocircumflex", "ocircumflexacute",
  "ocircumflexdotbelow", "ocircumflexgrave", "ocircumflexhookabove",
  "ocircumflextilde", "ocyrillic", "odblacute", "odblgrave", "odeva",
  "odieresis", "odieresiscyrillic", "odotbelow", "oe", "oekorean", "ogonek",
  "ogonekcmb", "ograve", "ogujarati", "oharmenian", "ohiragana", "ohookabove",
  "ohorn", "ohornacute", "ohorndotbelow", "ohorngrave", "ohornhookabove",
  "ohorntilde", "ohungarumlaut", "oi", "oinvertedbreve", "okatakana",
  "okatakanahalfwidth", "okorean", "olehebrew", "omacron", "omacronacute",
  "omacrongrave", "omdeva", "omega", "omega1", "omegacyrillic",
  "omegalatinclosed", "omegaroundcyrillic", "omegatitlocyrillic",
  "omegatonos", "omgujarati", "omicron", "omicrontonos", "omonospace", "one",
  "onearabic", "onebengali", "onecircle", "onecircleinversesansserif",
  "onedeva", "onedotenleader", "oneeighth", "onefitted", "onegujarati",
  "onegurmukhi", "onehackarabic", "onehalf", "onehangzhou",
  "oneideographicparen", "oneinferior", "onemonospace", "onenumeratorbengali",
  "oneoldstyle", "oneparen", "oneperiod", "onepersian", "onequarter",
  "oneroman", "onesuperior", "onethai", "onethird", "oogonek",
  "oogonekmacron", "oogurmukhi", "oomatragurmukhi", "oopen", "oparen",
  "openbullet", "option", "ordfeminine", "ordmasculine", "orthogonal",
  "oshortdeva", "oshortvowelsigndeva", "oslash", "oslashacute",
  "osmallhiragana", "osmallkatakana", "osmallkatakanahalfwidth",
  "ostrokeacute", "osuperior", "otcyrillic", "otilde", "otildeacute",
  "otildedieresis", "oubopomofo", "overline", "overlinecenterline",
  "overlinecmb", "overlinedashed", "overlinedblwavy", "overlinewavy",
  "overscore", "ovowelsignbengali", "ovowelsigndeva", "ovowelsigngujarati",
  "p", "paampssquare", "paasentosquare", "pabengali", "pacute", "padeva",
  "pagedown", "pageup", "pagujarati", "pagurmukhi", "pahiragana",
  "paiyannoithai", "pakatakana", "palatalizationcyrilliccmb",
  "palochkacyrillic", "pansioskorean", "paragraph", "parallel", "parenleft",
  "parenleftaltonearabic", "parenleftbt", "parenleftex", "parenleftinferior",
  "parenleftmonospace", "parenleftsmall", "parenleftsuperior", "parenlefttp",
  "parenleftvertical", "parenright", "parenrightaltonearabic", "parenrightbt",
  "parenrightex", "parenrightinferior", "parenrightmonospace",
  "parenrightsmall", "parenrightsuperior", "parenrighttp",
  "parenrightvertical", "partialdiff", "paseqhebrew", "pashtahebrew",
  "pasquare", "patah", "patah11", "patah1d", "patah2a", "patahhebrew",
  "patahnarrowhebrew", "patahquarterhebrew", "patahwidehebrew", "pazerhebrew",
  "pbopomofo", "pcircle", "pdotaccent", "pe", "pecyrillic", "pedagesh",
  "pedageshhebrew", "peezisquare", "pefinaldageshhebrew", "peharabic",
  "peharmenian", "pehebrew", "pehfinalarabic", "pehinitialarabic",
  "pehiragana", "pehmedialarabic", "pekatakana", "pemiddlehookcyrillic",
  "perafehebrew", "percent", "percentarabic", "percentmonospace",
  "percentsmall", "period", "periodarmenian", "periodcentered",
  "periodhalfwidth", "periodinferior", "periodmonospace", "periodsmall",
  "periodsuperior", "perispomenigreekcmb", "perpendicular", "perthousand",
  "peseta", "pfsquare", "phabengali", "phadeva", "phagujarati", "phagurmukhi",
  "phi", "phi1", "phieuphacirclekorean", "phieuphaparenkorean",
  "phieuphcirclekorean", "phieuphkorean", "phieuphparenkorean", "philatin",
  "phinthuthai", "phisymbolgreek", "phook", "phophanthai", "phophungthai",
  "phosamphaothai", "pi", "pieupacirclekorean", "pieupaparenkorean",
  "pieupcieuckorean", "pieupcirclekorean", "pieupkiyeokkorean", "pieupkorean",
  "pieupparenkorean", "pieupsioskiyeokkorean", "pieupsioskorean",
  "pieupsiostikeutkorean", "pieupthieuthkorean", "pieuptikeutkorean",
  "pihiragana", "pikatakana", "pisymbolgreek", "piwrarmenian", "plus",
  "plusbelowcmb", "pluscircle", "plusminus", "plusmod", "plusmonospace",
  "plussmall", "plussuperior", "pmonospace", "pmsquare", "pohiragana",
  "pointingindexdownwhite", "pointingindexleftwhite",
  "pointingindexrightwhite", "pointingindexupwhite", "pokatakana",
  "poplathai", "postalmark", "postalmarkface", "pparen", "precedes",
  "prescription", "primemod", "primereversed", "product", "projective",
  "prolongedkana", "propellor", "propersubset", "propersuperset",
  "proportion", "proportional", "psi", "psicyrillic",
  "psilipneumatacyrilliccmb", "pssquare", "puhiragana", "pukatakana",
  "pvsquare", "pwsquare", "q", "qadeva", "qadmahebrew", "qafarabic",
  "qaffinalarabic", "qafinitialarabic", "qafmedialarabic", "qamats",
  "qamats10", "qamats1a", "qamats1c", "qamats27", "qamats29", "qamats33",
  "qamatsde", "qamatshebrew", "qamatsnarrowhebrew", "qamatsqatanhebrew",
  "qamatsqatannarrowhebrew", "qamatsqatanquarterhebrew",
  "qamatsqatanwidehebrew", "qamatsquarterhebrew", "qamatswidehebrew",
  "qarneyparahebrew", "qbopomofo", "qcircle", "qhook", "qmonospace", "qof",
  "qofdagesh", "qofdageshhebrew", "qofhatafpatah", "qofhatafpatahhebrew",
  "qofhatafsegol", "qofhatafsegolhebrew", "qofhebrew", "qofhiriq",
  "qofhiriqhebrew", "qofholam", "qofholamhebrew", "qofpatah",
  "qofpatahhebrew", "qofqamats", "qofqamatshebrew", "qofqubuts",
  "qofqubutshebrew", "qofsegol", "qofsegolhebrew", "qofsheva",
  "qofshevahebrew", "qoftsere", "qoftserehebrew", "qparen", "quarternote",
  "qubuts", "qubuts18", "qubuts25", "qubuts31", "qubutshebrew",
  "qubutsnarrowhebrew", "qubutsquarterhebrew", "qubutswidehebrew", "question",
  "questionarabic", "questionarmenian", "questiondown", "questiondownsmall",
  "questiongreek", "questionmonospace", "questionsmall", "quotedbl",
  "quotedblbase", "quotedblleft", "quotedblmonospace", "quotedblprime",
  "quotedblprimereversed", "quotedblright", "quoteleft", "quoteleftreversed",
  "quotereversed", "quoteright", "quoterightn", "quotesinglbase",
  "quotesingle", "quotesinglemonospace", "r", "raarmenian", "rabengali",
  "racute", "radeva", "radical", "radicalex", "radoverssquare",
  "radoverssquaredsquare", "radsquare", "rafe", "rafehebrew", "ragujarati",
  "ragurmukhi", "rahiragana", "rakatakana", "rakatakanahalfwidth",
  "ralowerdiagonalbengali", "ramiddlediagonalbengali", "ramshorn", "ratio",
  "rbopomofo", "rcaron", "rcedilla", "rcircle", "rcommaaccent", "rdblgrave",
  "rdotaccent", "rdotbelow", "rdotbelowmacron", "referencemark",
  "reflexsubset", "reflexsuperset", "registered", "registersans",
  "registerserif", "reharabic", "reharmenian", "rehfinalarabic", "rehiragana",
  "rehyehaleflamarabic", "rekatakana", "rekatakanahalfwidth", "resh",
  "reshdageshhebrew", "reshhatafpatah", "reshhatafpatahhebrew",
  "reshhatafsegol", "reshhatafsegolhebrew", "reshhebrew", "reshhiriq",
  "reshhiriqhebrew", "reshholam", "reshholamhebrew", "reshpatah",
  "reshpatahhebrew", "reshqamats", "reshqamatshebrew", "reshqubuts",
  "reshqubutshebrew", "reshsegol", "reshsegolhebrew", "reshsheva",
  "reshshevahebrew", "reshtsere", "reshtserehebrew", "reversedtilde",
  "reviahebrew", "reviamugrashhebrew", "revlogicalnot", "rfishhook",
  "rfishhookreversed", "rhabengali", "rhadeva", "rho", "rhook", "rhookturned",
  "rhookturnedsuperior", "rhosymbolgreek", "rhotichookmod",
  "rieulacirclekorean", "rieulaparenkorean", "rieulcirclekorean",
  "rieulhieuhkorean", "rieulkiyeokkorean", "rieulkiyeoksioskorean",
  "rieulkorean", "rieulmieumkorean", "rieulpansioskorean", "rieulparenkorean",
  "rieulphieuphkorean", "rieulpieupkorean", "rieulpieupsioskorean",
  "rieulsioskorean", "rieulthieuthkorean", "rieultikeutkorean",
  "rieulyeorinhieuhkorean", "rightangle", "righttackbelowcmb",
  "righttriangle", "rihiragana", "rikatakana", "rikatakanahalfwidth", "ring",
  "ringbelowcmb", "ringcmb", "ringhalfleft", "ringhalfleftarmenian",
  "ringhalfleftbelowcmb", "ringhalfleftcentered", "ringhalfright",
  "ringhalfrightbelowcmb", "ringhalfrightcentered", "rinvertedbreve",
  "rittorusquare", "rlinebelow", "rlongleg", "rlonglegturned", "rmonospace",
  "rohiragana", "rokatakana", "rokatakanahalfwidth", "roruathai", "rparen",
  "rrabengali", "rradeva", "rragurmukhi", "rreharabic", "rrehfinalarabic",
  "rrvocalicbengali", "rrvocalicdeva", "rrvocalicgujarati",
  "rrvocalicvowelsignbengali", "rrvocalicvowelsigndeva",
  "rrvocalicvowelsigngujarati", "rsuperior", "rtblock", "rturned",
  "rturnedsuperior", "ruhiragana", "rukatakana", "rukatakanahalfwidth",
  "rupeemarkbengali", "rupeesignbengali", "rupiah", "ruthai",
  "rvocalicbengali", "rvocalicdeva", "rvocalicgujarati",
  "rvocalicvowelsignbengali", "rvocalicvowelsigndeva",
  "rvocalicvowelsigngujarati", "s", "sabengali", "sacute", "sacutedotaccent",
  "sadarabic", "sadeva", "sadfinalarabic", "sadinitialarabic",
  "sadmedialarabic", "sagujarati", "sagurmukhi", "sahiragana", "sakatakana",
  "sakatakanahalfwidth", "sallallahoualayhewasallamarabic", "samekh",
  "samekhdagesh", "samekhdageshhebrew", "samekhhebrew", "saraaathai",
  "saraaethai", "saraaimaimalaithai", "saraaimaimuanthai", "saraamthai",
  "saraathai", "saraethai", "saraiileftthai", "saraiithai", "saraileftthai",
  "saraithai", "saraothai", "saraueeleftthai", "saraueethai",
  "saraueleftthai", "sarauethai", "sarauthai", "sarauuthai", "sbopomofo",
  "scaron", "scarondotaccent", "scedilla", "schwa", "schwacyrillic",
  "schwadieresiscyrillic", "schwahook", "scircle", "scircumflex",
  "scommaaccent", "sdotaccent", "sdotbelow", "sdotbelowdotaccent",
  "seagullbelowcmb", "second", "secondtonechinese", "section", "seenarabic",
  "seenfinalarabic", "seeninitialarabic", "seenmedialarabic", "segol",
  "segol13", "segol1f", "segol2c", "segolhebrew", "segolnarrowhebrew",
  "segolquarterhebrew", "segoltahebrew", "segolwidehebrew", "seharmenian",
  "sehiragana", "sekatakana", "sekatakanahalfwidth", "semicolon",
  "semicolonarabic", "semicolonmonospace", "semicolonsmall",
  "semivoicedmarkkana", "semivoicedmarkkanahalfwidth", "sentisquare",
  "sentosquare", "seven", "sevenarabic", "sevenbengali", "sevencircle",
  "sevencircleinversesansserif", "sevendeva", "seveneighths", "sevengujarati",
  "sevengurmukhi", "sevenhackarabic", "sevenhangzhou",
  "sevenideographicparen", "seveninferior", "sevenmonospace", "sevenoldstyle",
  "sevenparen", "sevenperiod", "sevenpersian", "sevenroman", "sevensuperior",
  "seventeencircle", "seventeenparen", "seventeenperiod", "seventhai",
  "sfthyphen", "shaarmenian", "shabengali", "shacyrillic", "shaddaarabic",
  "shaddadammaarabic", "shaddadammatanarabic", "shaddafathaarabic",
  "shaddafathatanarabic", "shaddakasraarabic", "shaddakasratanarabic",
  "shade", "shadedark", "shadelight", "shademedium", "shadeva", "shagujarati",
  "shagurmukhi", "shalshelethebrew", "shbopomofo", "shchacyrillic",
  "sheenarabic", "sheenfinalarabic", "sheeninitialarabic",
  "sheenmedialarabic", "sheicoptic", "sheqel", "sheqelhebrew", "sheva",
  "sheva115", "sheva15", "sheva22", "sheva2e", "shevahebrew",
  "shevanarrowhebrew", "shevaquarterhebrew", "shevawidehebrew",
  "shhacyrillic", "shimacoptic", "shin", "shindagesh", "shindageshhebrew",
  "shindageshshindot", "shindageshshindothebrew", "shindageshsindot",
  "shindageshsindothebrew", "shindothebrew", "shinhebrew", "shinshindot",
  "shinshindothebrew", "shinsindot", "shinsindothebrew", "shook", "sigma",
  "sigma1", "sigmafinal", "sigmalunatesymbolgreek", "sihiragana",
  "sikatakana", "sikatakanahalfwidth", "siluqhebrew", "siluqlefthebrew",
  "similar", "sindothebrew", "siosacirclekorean", "siosaparenkorean",
  "sioscieuckorean", "sioscirclekorean", "sioskiyeokkorean", "sioskorean",
  "siosnieunkorean", "siosparenkorean", "siospieupkorean", "siostikeutkorean",
  "six", "sixarabic", "sixbengali", "sixcircle", "sixcircleinversesansserif",
  "sixdeva", "sixgujarati", "sixgurmukhi", "sixhackarabic", "sixhangzhou",
  "sixideographicparen", "sixinferior", "sixmonospace", "sixoldstyle",
  "sixparen", "sixperiod", "sixpersian", "sixroman", "sixsuperior",
  "sixteencircle", "sixteencurrencydenominatorbengali", "sixteenparen",
  "sixteenperiod", "sixthai", "slash", "slashmonospace", "slong",
  "slongdotaccent", "smileface", "smonospace", "sofpasuqhebrew", "softhyphen",
  "softsigncyrillic", "sohiragana", "sokatakana", "sokatakanahalfwidth",
  "soliduslongoverlaycmb", "solidusshortoverlaycmb", "sorusithai",
  "sosalathai", "sosothai", "sosuathai", "space", "spacehackarabic", "spade",
  "spadesuitblack", "spadesuitwhite", "sparen", "squarebelowcmb", "squarecc",
  "squarecm", "squarediagonalcrosshatchfill", "squarehorizontalfill",
  "squarekg", "squarekm", "squarekmcapital", "squareln", "squarelog",
  "squaremg", "squaremil", "squaremm", "squaremsquared",
  "squareorthogonalcrosshatchfill", "squareupperlefttolowerrightfill",
  "squareupperrighttolowerleftfill", "squareverticalfill",
  "squarewhitewithsmallblack", "srsquare", "ssabengali", "ssadeva",
  "ssagujarati", "ssangcieuckorean", "ssanghieuhkorean", "ssangieungkorean",
  "ssangkiyeokkorean", "ssangnieunkorean", "ssangpieupkorean",
  "ssangsioskorean", "ssangtikeutkorean", "ssuperior", "sterling",
  "sterlingmonospace", "strokelongoverlaycmb", "strokeshortoverlaycmb",
  "subset", "subsetnotequal", "subsetorequal", "succeeds", "suchthat",
  "suhiragana", "sukatakana", "sukatakanahalfwidth", "sukunarabic",
  "summation", "sun", "superset", "supersetnotequal", "supersetorequal",
  "svsquare", "syouwaerasquare", "t", "tabengali", "tackdown", "tackleft",
  "tadeva", "tagujarati", "tagurmukhi", "taharabic", "tahfinalarabic",
  "tahinitialarabic", "tahiragana", "tahmedialarabic", "taisyouerasquare",
  "takatakana", "takatakanahalfwidth", "tatweelarabic", "tau", "tav",
  "tavdages", "tavdagesh", "tavdageshhebrew", "tavhebrew", "tbar",
  "tbopomofo", "tcaron", "tccurl", "tcedilla", "tcheharabic",
  "tchehfinalarabic", "tchehinitialarabic", "tchehmedialarabic",
  "tchehmeeminitialarabic", "tcircle", "tcircumflexbelow", "tcommaaccent",
  "tdieresis", "tdotaccent", "tdotbelow", "tecyrillic", "tedescendercyrillic",
  "teharabic", "tehfinalarabic", "tehhahinitialarabic",
  "tehhahisolatedarabic", "tehinitialarabic", "tehiragana",
  "tehjeeminitialarabic", "tehjeemisolatedarabic", "tehmarbutaarabic",
  "tehmarbutafinalarabic", "tehmedialarabic", "tehmeeminitialarabic",
  "tehmeemisolatedarabic", "tehnoonfinalarabic", "tekatakana",
  "tekatakanahalfwidth", "telephone", "telephoneblack", "telishagedolahebrew",
  "telishaqetanahebrew", "tencircle", "tenideographicparen", "tenparen",
  "tenperiod", "tenroman", "tesh", "tet", "tetdagesh", "tetdageshhebrew",
  "tethebrew", "tetsecyrillic", "tevirhebrew", "tevirlefthebrew",
  "thabengali", "thadeva", "thagujarati", "thagurmukhi", "thalarabic",
  "thalfinalarabic", "thanthakhatlowleftthai", "thanthakhatlowrightthai",
  "thanthakhatthai", "thanthakhatupperleftthai", "theharabic",
  "thehfinalarabic", "thehinitialarabic", "thehmedialarabic", "thereexists",
  "therefore", "theta", "theta1", "thetasymbolgreek", "thieuthacirclekorean",
  "thieuthaparenkorean", "thieuthcirclekorean", "thieuthkorean",
  "thieuthparenkorean", "thirteencircle", "thirteenparen", "thirteenperiod",
  "thonangmonthothai", "thook", "thophuthaothai", "thorn", "thothahanthai",
  "thothanthai", "thothongthai", "thothungthai", "thousandcyrillic",
  "thousandsseparatorarabic", "thousandsseparatorpersian", "three",
  "threearabic", "threebengali", "threecircle", "threecircleinversesansserif",
  "threedeva", "threeeighths", "threegujarati", "threegurmukhi",
  "threehackarabic", "threehangzhou", "threeideographicparen",
  "threeinferior", "threemonospace", "threenumeratorbengali", "threeoldstyle",
  "threeparen", "threeperiod", "threepersian", "threequarters",
  "threequartersemdash", "threeroman", "threesuperior", "threethai",
  "thzsquare", "tihiragana", "tikatakana", "tikatakanahalfwidth",
  "tikeutacirclekorean", "tikeutaparenkorean", "tikeutcirclekorean",
  "tikeutkorean", "tikeutparenkorean", "tilde", "tildebelowcmb", "tildecmb",
  "tildecomb", "tildedoublecmb", "tildeoperator", "tildeoverlaycmb",
  "tildeverticalcmb", "timescircle", "tipehahebrew", "tipehalefthebrew",
  "tippigurmukhi", "titlocyrilliccmb", "tiwnarmenian", "tlinebelow",
  "tmonospace", "toarmenian", "tohiragana", "tokatakana",
  "tokatakanahalfwidth", "tonebarextrahighmod", "tonebarextralowmod",
  "tonebarhighmod", "tonebarlowmod", "tonebarmidmod", "tonefive", "tonesix",
  "tonetwo", "tonos", "tonsquare", "topatakthai", "tortoiseshellbracketleft",
  "tortoiseshellbracketleftsmall", "tortoiseshellbracketleftvertical",
  "tortoiseshellbracketright", "tortoiseshellbracketrightsmall",
  "tortoiseshellbracketrightvertical", "totaothai", "tpalatalhook", "tparen",
  "trademark", "trademarksans", "trademarkserif", "tretroflexhook", "triagdn",
  "triaglf", "triagrt", "triagup", "ts", "tsadi", "tsadidagesh",
  "tsadidageshhebrew", "tsadihebrew", "tsecyrillic", "tsere", "tsere12",
  "tsere1e", "tsere2b", "tserehebrew", "tserenarrowhebrew",
  "tserequarterhebrew", "tserewidehebrew", "tshecyrillic", "tsuperior",
  "ttabengali", "ttadeva", "ttagujarati", "ttagurmukhi", "tteharabic",
  "ttehfinalarabic", "ttehinitialarabic", "ttehmedialarabic", "tthabengali",
  "tthadeva", "tthagujarati", "tthagurmukhi", "tturned", "tuhiragana",
  "tukatakana", "tukatakanahalfwidth", "tusmallhiragana", "tusmallkatakana",
  "tusmallkatakanahalfwidth", "twelvecircle", "twelveparen", "twelveperiod",
  "twelveroman", "twentycircle", "twentyhangzhou", "twentyparen",
  "twentyperiod", "two", "twoarabic", "twobengali", "twocircle",
  "twocircleinversesansserif", "twodeva", "twodotenleader", "twodotleader",
  "twodotleadervertical", "twogujarati", "twogurmukhi", "twohackarabic",
  "twohangzhou", "twoideographicparen", "twoinferior", "twomonospace",
  "twonumeratorbengali", "twooldstyle", "twoparen", "twoperiod", "twopersian",
  "tworoman", "twostroke", "twosuperior", "twothai", "twothirds", "u",
  "uacute", "ubar", "ubengali", "ubopomofo", "ubreve", "ucaron", "ucircle",
  "ucircumflex", "ucircumflexbelow", "ucyrillic", "udattadeva", "udblacute",
  "udblgrave", "udeva", "udieresis", "udieresisacute", "udieresisbelow",
  "udieresiscaron", "udieresiscyrillic", "udieresisgrave", "udieresismacron",
  "udotbelow", "ugrave", "ugujarati", "ugurmukhi", "uhiragana", "uhookabove",
  "uhorn", "uhornacute", "uhorndotbelow", "uhorngrave", "uhornhookabove",
  "uhorntilde", "uhungarumlaut", "uhungarumlautcyrillic", "uinvertedbreve",
  "ukatakana", "ukatakanahalfwidth", "ukcyrillic", "ukorean", "umacron",
  "umacroncyrillic", "umacrondieresis", "umatragurmukhi", "umonospace",
  "underscore", "underscoredbl", "underscoremonospace", "underscorevertical",
  "underscorewavy", "union", "universal", "uogonek", "uparen", "upblock",
  "upperdothebrew", "upsilon", "upsilondieresis", "upsilondieresistonos",
  "upsilonlatin", "upsilontonos", "uptackbelowcmb", "uptackmod",
  "uragurmukhi", "uring", "ushortcyrillic", "usmallhiragana",
  "usmallkatakana", "usmallkatakanahalfwidth", "ustraightcyrillic",
  "ustraightstrokecyrillic", "utilde", "utildeacute", "utildebelow",
  "uubengali", "uudeva", "uugujarati", "uugurmukhi", "uumatragurmukhi",
  "uuvowelsignbengali", "uuvowelsigndeva", "uuvowelsigngujarati",
  "uvowelsignbengali", "uvowelsigndeva", "uvowelsigngujarati", "v", "vadeva",
  "vagujarati", "vagurmukhi", "vakatakana", "vav", "vavdagesh", "vavdagesh65",
  "vavdageshhebrew", "vavhebrew", "vavholam", "vavholamhebrew",
  "vavvavhebrew", "vavyodhebrew", "vcircle", "vdotbelow", "vecyrillic",
  "veharabic", "vehfinalarabic", "vehinitialarabic", "vehmedialarabic",
  "vekatakana", "venus", "verticalbar", "verticallineabovecmb",
  "verticallinebelowcmb", "verticallinelowmod", "verticallinemod",
  "vewarmenian", "vhook", "vikatakana", "viramabengali", "viramadeva",
  "viramagujarati", "visargabengali", "visargadeva", "visargagujarati",
  "vmonospace", "voarmenian", "voicediterationhiragana",
  "voicediterationkatakana", "voicedmarkkana", "voicedmarkkanahalfwidth",
  "vokatakana", "vparen", "vtilde", "vturned", "vuhiragana", "vukatakana",
  "w", "wacute", "waekorean", "wahiragana", "wakatakana",
  "wakatakanahalfwidth", "wakorean", "wasmallhiragana", "wasmallkatakana",
  "wattosquare", "wavedash", "wavyunderscorevertical", "wawarabic",
  "wawfinalarabic", "wawhamzaabovearabic", "wawhamzaabovefinalarabic",
  "wbsquare", "wcircle", "wcircumflex", "wdieresis", "wdotaccent",
  "wdotbelow", "wehiragana", "weierstrass", "wekatakana", "wekorean",
  "weokorean", "wgrave", "whitebullet", "whitecircle", "whitecircleinverse",
  "whitecornerbracketleft", "whitecornerbracketleftvertical",
  "whitecornerbracketright", "whitecornerbracketrightvertical",
  "whitediamond", "whitediamondcontainingblacksmalldiamond",
  "whitedownpointingsmalltriangle", "whitedownpointingtriangle",
  "whiteleftpointingsmalltriangle", "whiteleftpointingtriangle",
  "whitelenticularbracketleft", "whitelenticularbracketright",
  "whiterightpointingsmalltriangle", "whiterightpointingtriangle",
  "whitesmallsquare", "whitesmilingface", "whitesquare", "whitestar",
  "whitetelephone", "whitetortoiseshellbracketleft",
  "whitetortoiseshellbracketright", "whiteuppointingsmalltriangle",
  "whiteuppointingtriangle", "wihiragana", "wikatakana", "wikorean",
  "wmonospace", "wohiragana", "wokatakana", "wokatakanahalfwidth", "won",
  "wonmonospace", "wowaenthai", "wparen", "wring", "wsuperior", "wturned",
  "wynn", "x", "xabovecmb", "xbopomofo", "xcircle", "xdieresis", "xdotaccent",
  "xeharmenian", "xi", "xmonospace", "xparen", "xsuperior", "y",
  "yaadosquare", "yabengali", "yacute", "yadeva", "yaekorean", "yagujarati",
  "yagurmukhi", "yahiragana", "yakatakana", "yakatakanahalfwidth", "yakorean",
  "yamakkanthai", "yasmallhiragana", "yasmallkatakana",
  "yasmallkatakanahalfwidth", "yatcyrillic", "ycircle", "ycircumflex",
  "ydieresis", "ydotaccent", "ydotbelow", "yeharabic", "yehbarreearabic",
  "yehbarreefinalarabic", "yehfinalarabic", "yehhamzaabovearabic",
  "yehhamzaabovefinalarabic", "yehhamzaaboveinitialarabic",
  "yehhamzaabovemedialarabic", "yehinitialarabic", "yehmedialarabic",
  "yehmeeminitialarabic", "yehmeemisolatedarabic", "yehnoonfinalarabic",
  "yehthreedotsbelowarabic", "yekorean", "yen", "yenmonospace", "yeokorean",
  "yeorinhieuhkorean", "yerahbenyomohebrew", "yerahbenyomolefthebrew",
  "yericyrillic", "yerudieresiscyrillic", "yesieungkorean",
  "yesieungpansioskorean", "yesieungsioskorean", "yetivhebrew", "ygrave",
  "yhook", "yhookabove", "yiarmenian", "yicyrillic", "yikorean", "yinyang",
  "yiwnarmenian", "ymonospace", "yod", "yoddagesh", "yoddageshhebrew",
  "yodhebrew", "yodyodhebrew", "yodyodpatahhebrew", "yohiragana", "yoikorean",
  "yokatakana", "yokatakanahalfwidth", "yokorean", "yosmallhiragana",
  "yosmallkatakana", "yosmallkatakanahalfwidth", "yotgreek", "yoyaekorean",
  "yoyakorean", "yoyakthai", "yoyingthai", "yparen", "ypogegrammeni",
  "ypogegrammenigreekcmb", "yr", "yring", "ysuperior", "ytilde", "yturned",
  "yuhiragana", "yuikorean", "yukatakana", "yukatakanahalfwidth", "yukorean",
  "yusbigcyrillic", "yusbigiotifiedcyrillic", "yuslittlecyrillic",
  "yuslittleiotifiedcyrillic", "yusmallhiragana", "yusmallkatakana",
  "yusmallkatakanahalfwidth", "yuyekorean", "yuyeokorean", "yyabengali",
  "yyadeva", "z", "zaarmenian", "zacute", "zadeva", "zagurmukhi", "zaharabic",
  "zahfinalarabic", "zahinitialarabic", "zahiragana", "zahmedialarabic",
  "zainarabic", "zainfinalarabic", "zakatakana", "zaqefgadolhebrew",
  "zaqefqatanhebrew", "zarqahebrew", "zayin", "zayindagesh",
  "zayindageshhebrew", "zayinhebrew", "zbopomofo", "zcaron", "zcircle",
  "zcircumflex", "zcurl", "zdot", "zdotaccent", "zdotbelow", "zecyrillic",
  "zedescendercyrillic", "zedieresiscyrillic", "zehiragana", "zekatakana",
  "zero", "zeroarabic", "zerobengali", "zerodeva", "zerogujarati",
  "zerogurmukhi", "zerohackarabic", "zeroinferior", "zeromonospace",
  "zerooldstyle", "zeropersian", "zerosuperior", "zerothai",
  "zerowidthjoiner", "zerowidthnonjoiner", "zerowidthspace", "zeta",
  "zhbopomofo", "zhearmenian", "zhebrevecyrillic", "zhecyrillic",
  "zhedescendercyrillic", "zhedieresiscyrillic", "zihiragana", "zikatakana",
  "zinorhebrew", "zlinebelow", "zmonospace", "zohiragana", "zokatakana",
  "zparen", "zretroflexhook", "zstroke", "zuhiragana", "zukatakana", NULL
};


static const char *glyph_values[] = {
  "\x02\x41\x00", "\x02\xc6\x00", "\x02\xfc\x01", "\x02\xe2\x01",
  "\x02\xe6\xf7", "\x02\xc1\x00", "\x02\xe1\xf7", "\x02\x02\x01",
  "\x02\xae\x1e", "\x02\xd0\x04", "\x02\xb6\x1e", "\x02\xb0\x1e",
  "\x02\xb2\x1e", "\x02\xb4\x1e", "\x02\xcd\x01", "\x02\xb6\x24",
  "\x02\xc2\x00", "\x02\xa4\x1e", "\x02\xac\x1e", "\x02\xa6\x1e",
  "\x02\xa8\x1e", "\x02\xe2\xf7", "\x02\xaa\x1e", "\x02\xc9\xf6",
  "\x02\xb4\xf7", "\x02\x10\x04", "\x02\x00\x02", "\x02\xc4\x00",
  "\x02\xd2\x04", "\x02\xde\x01", "\x02\xe4\xf7", "\x02\xa0\x1e",
  "\x02\xe0\x01", "\x02\xc0\x00", "\x02\xe0\xf7", "\x02\xa2\x1e",
  "\x02\xd4\x04", "\x02\x02\x02", "\x02\x91\x03", "\x02\x86\x03",
  "\x02\x00\x01", "\x02\x21\xff", "\x02\x04\x01", "\x02\xc5\x00",
  "\x02\xfa\x01", "\x02\x00\x1e", "\x02\xe5\xf7", "\x02\x61\xf7",
  "\x02\xc3\x00", "\x02\xe3\xf7", "\x02\x31\x05", "\x02\x42\x00",
  "\x02\xb7\x24", "\x02\x02\x1e", "\x02\x04\x1e", "\x02\x11\x04",
  "\x02\x32\x05", "\x02\x92\x03", "\x02\x81\x01", "\x02\x06\x1e",
  "\x02\x22\xff", "\x02\xf4\xf6", "\x02\x62\xf7", "\x02\x82\x01",
  "\x02\x43\x00", "\x02\x3e\x05", "\x02\x06\x01", "\x02\xca\xf6",
  "\x02\xf5\xf6", "\x02\x0c\x01", "\x02\xc7\x00", "\x02\x08\x1e",
  "\x02\xe7\xf7", "\x02\xb8\x24", "\x02\x08\x01", "\x02\x0a\x01",
  "\x02\x0a\x01", "\x02\xb8\xf7", "\x02\x49\x05", "\x02\xbc\x04",
  "\x02\x27\x04", "\x02\xbe\x04", "\x02\xb6\x04", "\x02\xf4\x04",
  "\x02\x43\x05", "\x02\xcb\x04", "\x02\xb8\x04", "\x02\xa7\x03",
  "\x02\x87\x01", "\x02\xf6\xf6", "\x02\x23\xff", "\x02\x51\x05",
  "\x02\x63\xf7", "\x02\x44\x00", "\x02\xf1\x01", "\x02\xc4\x01",
  "\x02\x34\x05", "\x02\x89\x01", "\x02\x0e\x01", "\x02\x10\x1e",
  "\x02\xb9\x24", "\x02\x12\x1e", "\x02\x10\x01", "\x02\x0a\x1e",
  "\x02\x0c\x1e", "\x02\x14\x04", "\x02\xee\x03", "\x02\x06\x22",
  "\x02\x94\x03", "\x02\x8a\x01", "\x02\xcb\xf6", "\x02\xcc\xf6",
  "\x02\xcd\xf6", "\x02\xa8\xf7", "\x02\xdc\x03", "\x02\x02\x04",
  "\x02\x0e\x1e", "\x02\x24\xff", "\x02\xf7\xf6", "\x02\x10\x01",
  "\x02\x64\xf7", "\x02\x8b\x01", "\x02\xf2\x01", "\x02\xc5\x01",
  "\x02\xe0\x04", "\x02\x05\x04", "\x02\x0f\x04", "\x02\x45\x00",
  "\x02\xc9\x00", "\x02\xe9\xf7", "\x02\x14\x01", "\x02\x1a\x01",
  "\x02\x1c\x1e", "\x02\x35\x05", "\x02\xba\x24", "\x02\xca\x00",
  "\x02\xbe\x1e", "\x02\x18\x1e", "\x02\xc6\x1e", "\x02\xc0\x1e",
  "\x02\xc2\x1e", "\x02\xea\xf7", "\x02\xc4\x1e", "\x02\x04\x04",
  "\x02\x04\x02", "\x02\xcb\x00", "\x02\xeb\xf7", "\x02\x16\x01",
  "\x02\x16\x01", "\x02\xb8\x1e", "\x02\x24\x04", "\x02\xc8\x00",
  "\x02\xe8\xf7", "\x02\x37\x05", "\x02\xba\x1e", "\x02\x67\x21",
  "\x02\x06\x02", "\x02\x64\x04", "\x02\x1b\x04", "\x02\x6a\x21",
  "\x02\x12\x01", "\x02\x16\x1e", "\x02\x14\x1e", "\x02\x1c\x04",
  "\x02\x25\xff", "\x02\x1d\x04", "\x02\xa2\x04", "\x02\x4a\x01",
  "\x02\xa4\x04", "\x02\xc7\x04", "\x02\x18\x01", "\x02\x90\x01",
  "\x02\x95\x03", "\x02\x88\x03", "\x02\x20\x04", "\x02\x8e\x01",
  "\x02\x2d\x04", "\x02\x21\x04", "\x02\xaa\x04", "\x02\xa9\x01",
  "\x02\x65\xf7", "\x02\x97\x03", "\x02\x38\x05", "\x02\x89\x03",
  "\x02\xd0\x00", "\x02\xf0\xf7", "\x02\xbc\x1e", "\x02\x1a\x1e",
  "\x02\xac\x20", "\x02\xb7\x01", "\x02\xee\x01", "\x02\xb8\x01",
  "\x02\x46\x00", "\x02\xbb\x24", "\x02\x1e\x1e", "\x02\x56\x05",
  "\x02\xe4\x03", "\x02\x91\x01", "\x02\x72\x04", "\x02\x64\x21",
  "\x02\x26\xff", "\x02\x63\x21", "\x02\x66\xf7", "\x02\x47\x00",
  "\x02\x87\x33", "\x02\xf4\x01", "\x02\x93\x03", "\x02\x94\x01",
  "\x02\xea\x03", "\x02\x1e\x01", "\x02\xe6\x01", "\x02\x22\x01",
  "\x02\xbc\x24", "\x02\x1c\x01", "\x02\x22\x01", "\x02\x20\x01",
  "\x02\x20\x01", "\x02\x13\x04", "\x02\x42\x05", "\x02\x94\x04",
  "\x02\x92\x04", "\x02\x90\x04", "\x02\x93\x01", "\x02\x33\x05",
  "\x02\x03\x04", "\x02\x20\x1e", "\x02\x27\xff", "\x02\xce\xf6",
  "\x02\x60\xf7", "\x02\x67\xf7", "\x02\x9b\x02", "\x02\xe4\x01",
  "\x02\x48\x00", "\x02\xcf\x25", "\x02\xaa\x25", "\x02\xab\x25",
  "\x02\xa1\x25", "\x02\xcb\x33", "\x02\xa8\x04", "\x02\xb2\x04",
  "\x02\x2a\x04", "\x02\x26\x01", "\x02\x2a\x1e", "\x02\x28\x1e",
  "\x02\xbd\x24", "\x02\x24\x01", "\x02\x26\x1e", "\x02\x22\x1e",
  "\x02\x24\x1e", "\x02\x28\xff", "\x02\x40\x05", "\x02\xe8\x03",
  "\x02\x68\xf7", "\x02\xcf\xf6", "\x02\xf8\xf6", "\x02\x90\x33",
  "\x02\x49\x00", "\x02\x2f\x04", "\x02\x32\x01", "\x02\x2e\x04",
  "\x02\xcd\x00", "\x02\xed\xf7", "\x02\x2c\x01", "\x02\xcf\x01",
  "\x02\xbe\x24", "\x02\xce\x00", "\x02\xee\xf7", "\x02\x06\x04",
  "\x02\x08\x02", "\x02\xcf\x00", "\x02\x2e\x1e", "\x02\xe4\x04",
  "\x02\xef\xf7", "\x02\x30\x01", "\x02\x30\x01", "\x02\xca\x1e",
  "\x02\xd6\x04", "\x02\x15\x04", "\x02\x11\x21", "\x02\xcc\x00",
  "\x02\xec\xf7", "\x02\xc8\x1e", "\x02\x18\x04", "\x02\x0a\x02",
  "\x02\x19\x04", "\x02\x2a\x01", "\x02\xe2\x04", "\x02\x29\xff",
  "\x02\x3b\x05", "\x02\x01\x04", "\x02\x2e\x01", "\x02\x99\x03",
  "\x02\x96\x01", "\x02\xaa\x03", "\x02\x8a\x03", "\x02\x69\xf7",
  "\x02\x97\x01", "\x02\x28\x01", "\x02\x2c\x1e", "\x02\x74\x04",
  "\x02\x76\x04", "\x02\x4a\x00", "\x02\x41\x05", "\x02\xbf\x24",
  "\x02\x34\x01", "\x02\x08\x04", "\x02\x4b\x05", "\x02\x2a\xff",
  "\x02\x6a\xf7", "\x02\x4b\x00", "\x02\x85\x33", "\x02\xcd\x33",
  "\x02\xa0\x04", "\x02\x30\x1e", "\x02\x1a\x04", "\x02\x9a\x04",
  "\x02\xc3\x04", "\x02\x9a\x03", "\x02\x9e\x04", "\x02\x9c\x04",
  "\x02\xe8\x01", "\x02\x36\x01", "\x02\xc0\x24", "\x02\x36\x01",
  "\x02\x32\x1e", "\x02\x54\x05", "\x02\x3f\x05", "\x02\x25\x04",
  "\x02\xe6\x03", "\x02\x98\x01", "\x02\x0c\x04", "\x02\x34\x1e",
  "\x02\x2b\xff", "\x02\x80\x04", "\x02\xde\x03", "\x02\x6e\x04",
  "\x02\x6b\xf7", "\x02\x4c\x00", "\x02\xc7\x01", "\x02\xbf\xf6",
  "\x02\x39\x01", "\x02\x9b\x03", "\x02\x3d\x01", "\x02\x3b\x01",
  "\x02\xc1\x24", "\x02\x3c\x1e", "\x02\x3b\x01", "\x02\x3f\x01",
  "\x02\x3f\x01", "\x02\x36\x1e", "\x02\x38\x1e", "\x02\x3c\x05",
  "\x02\xc8\x01", "\x02\x09\x04", "\x02\x3a\x1e", "\x02\x2c\xff",
  "\x02\x41\x01", "\x02\xf9\xf6", "\x02\x6c\xf7", "\x02\x4d\x00",
  "\x02\x86\x33", "\x02\xd0\xf6", "\x02\xaf\xf7", "\x02\x3e\x1e",
  "\x02\xc2\x24", "\x02\x40\x1e", "\x02\x42\x1e", "\x02\x44\x05",
  "\x02\x2d\xff", "\x02\x6d\xf7", "\x02\x9c\x01", "\x02\x9c\x03",
  "\x02\x4e\x00", "\x02\xca\x01", "\x02\x43\x01", "\x02\x47\x01",
  "\x02\x45\x01", "\x02\xc3\x24", "\x02\x4a\x1e", "\x02\x45\x01",
  "\x02\x44\x1e", "\x02\x46\x1e", "\x02\x9d\x01", "\x02\x68\x21",
  "\x02\xcb\x01", "\x02\x0a\x04", "\x02\x48\x1e", "\x02\x2e\xff",
  "\x02\x46\x05", "\x02\x6e\xf7", "\x02\xd1\x00", "\x02\xf1\xf7",
  "\x02\x9d\x03", "\x02\x4f\x00", "\x02\x52\x01", "\x02\xfa\xf6",
  "\x02\xd3\x00", "\x02\xf3\xf7", "\x02\xe8\x04", "\x02\xea\x04",
  "\x02\x4e\x01", "\x02\xd1\x01", "\x02\x9f\x01", "\x02\xc4\x24",
  "\x02\xd4\x00", "\x02\xd0\x1e", "\x02\xd8\x1e", "\x02\xd2\x1e",
  "\x02\xd4\x1e", "\x02\xf4\xf7", "\x02\xd6\x1e", "\x02\x1e\x04",
  "\x02\x50\x01", "\x02\x0c\x02", "\x02\xd6\x00", "\x02\xe6\x04",
  "\x02\xf6\xf7", "\x02\xcc\x1e", "\x02\xfb\xf6", "\x02\xd2\x00",
  "\x02\xf2\xf7", "\x02\x55\x05", "\x02\x26\x21", "\x02\xce\x1e",
  "\x02\xa0\x01", "\x02\xda\x1e", "\x02\xe2\x1e", "\x02\xdc\x1e",
  "\x02\xde\x1e", "\x02\xe0\x1e", "\x02\x50\x01", "\x02\xa2\x01",
  "\x02\x0e\x02", "\x02\x4c\x01", "\x02\x52\x1e", "\x02\x50\x1e",
  "\x02\x26\x21", "\x02\x60\x04", "\x02\xa9\x03", "\x02\x7a\x04",
  "\x02\x7c\x04", "\x02\x8f\x03", "\x02\x9f\x03", "\x02\x8c\x03",
  "\x02\x2f\xff", "\x02\x60\x21", "\x02\xea\x01", "\x02\xec\x01",
  "\x02\x86\x01", "\x02\xd8\x00", "\x02\xfe\x01", "\x02\xf8\xf7",
  "\x02\x6f\xf7", "\x02\xfe\x01", "\x02\x7e\x04", "\x02\xd5\x00",
  "\x02\x4c\x1e", "\x02\x4e\x1e", "\x02\xf5\xf7", "\x02\x50\x00",
  "\x02\x54\x1e", "\x02\xc5\x24", "\x02\x56\x1e", "\x02\x1f\x04",
  "\x02\x4a\x05", "\x02\xa6\x04", "\x02\xa6\x03", "\x02\xa4\x01",
  "\x02\xa0\x03", "\x02\x53\x05", "\x02\x30\xff", "\x02\xa8\x03",
  "\x02\x70\x04", "\x02\x70\xf7", "\x02\x51\x00", "\x02\xc6\x24",
  "\x02\x31\xff", "\x02\x71\xf7", "\x02\x52\x00", "\x02\x4c\x05",
  "\x02\x54\x01", "\x02\x58\x01", "\x02\x56\x01", "\x02\xc7\x24",
  "\x02\x56\x01", "\x02\x10\x02", "\x02\x58\x1e", "\x02\x5a\x1e",
  "\x02\x5c\x1e", "\x02\x50\x05", "\x02\x1c\x21", "\x02\xa1\x03",
  "\x02\xfc\xf6", "\x02\x12\x02", "\x02\x5e\x1e", "\x02\x32\xff",
  "\x02\x72\xf7", "\x02\x81\x02", "\x02\xb6\x02", "\x02\x53\x00",
  "\x02\x0c\x25", "\x02\x14\x25", "\x02\x10\x25", "\x02\x18\x25",
  "\x02\x3c\x25", "\x02\x2c\x25", "\x02\x34\x25", "\x02\x1c\x25",
  "\x02\x24\x25", "\x02\x00\x25", "\x02\x02\x25", "\x02\x61\x25",
  "\x02\x62\x25", "\x02\x56\x25", "\x02\x55\x25", "\x02\x63\x25",
  "\x02\x51\x25", "\x02\x57\x25", "\x02\x5d\x25", "\x02\x5c\x25",
  "\x02\x5b\x25", "\x02\x5e\x25", "\x02\x5f\x25", "\x02\x5a\x25",
  "\x02\x54\x25", "\x02\x69\x25", "\x02\x66\x25", "\x02\x60\x25",
  "\x02\x50\x25", "\x02\x6c\x25", "\x02\x67\x25", "\x02\x68\x25",
  "\x02\x64\x25", "\x02\x65\x25", "\x02\x59\x25", "\x02\x58\x25",
  "\x02\x52\x25", "\x02\x53\x25", "\x02\x6b\x25", "\x02\x6a\x25",
  "\x02\x5a\x01", "\x02\x64\x1e", "\x02\xe0\x03", "\x02\x60\x01",
  "\x02\x66\x1e", "\x02\xfd\xf6", "\x02\x5e\x01", "\x02\x8f\x01",
  "\x02\xd8\x04", "\x02\xda\x04", "\x02\xc8\x24", "\x02\x5c\x01",
  "\x02\x18\x02", "\x02\x60\x1e", "\x02\x62\x1e", "\x02\x68\x1e",
  "\x02\x4d\x05", "\x02\x66\x21", "\x02\x47\x05", "\x02\x28\x04",
  "\x02\x29\x04", "\x02\xe2\x03", "\x02\xba\x04", "\x02\xec\x03",
  "\x02\xa3\x03", "\x02\x65\x21", "\x02\x33\xff", "\x02\x2c\x04",
  "\x02\x73\xf7", "\x02\xda\x03", "\x02\x54\x00", "\x02\xa4\x03",
  "\x02\x66\x01", "\x02\x64\x01", "\x02\x62\x01", "\x02\xc9\x24",
  "\x02\x70\x1e", "\x02\x62\x01", "\x02\x6a\x1e", "\x02\x6c\x1e",
  "\x02\x22\x04", "\x02\xac\x04", "\x02\x69\x21", "\x02\xb4\x04",
  "\x02\x98\x03", "\x02\xac\x01", "\x02\xde\x00", "\x02\xfe\xf7",
  "\x02\x62\x21", "\x02\xfe\xf6", "\x02\x4f\x05", "\x02\x6e\x1e",
  "\x02\x34\xff", "\x02\x39\x05", "\x02\xbc\x01", "\x02\x84\x01",
  "\x02\xa7\x01", "\x02\xae\x01", "\x02\x26\x04", "\x02\x0b\x04",
  "\x02\x74\xf7", "\x02\x6b\x21", "\x02\x61\x21", "\x02\x55\x00",
  "\x02\xda\x00", "\x02\xfa\xf7", "\x02\x6c\x01", "\x02\xd3\x01",
  "\x02\xca\x24", "\x02\xdb\x00", "\x02\x76\x1e", "\x02\xfb\xf7",
  "\x02\x23\x04", "\x02\x70\x01", "\x02\x14\x02", "\x02\xdc\x00",
  "\x02\xd7\x01", "\x02\x72\x1e", "\x02\xd9\x01", "\x02\xf0\x04",
  "\x02\xdb\x01", "\x02\xd5\x01", "\x02\xfc\xf7", "\x02\xe4\x1e",
  "\x02\xd9\x00", "\x02\xf9\xf7", "\x02\xe6\x1e", "\x02\xaf\x01",
  "\x02\xe8\x1e", "\x02\xf0\x1e", "\x02\xea\x1e", "\x02\xec\x1e",
  "\x02\xee\x1e", "\x02\x70\x01", "\x02\xf2\x04", "\x02\x16\x02",
  "\x02\x78\x04", "\x02\x6a\x01", "\x02\xee\x04", "\x02\x7a\x1e",
  "\x02\x35\xff", "\x02\x72\x01", "\x02\xa5\x03", "\x02\xd2\x03",
  "\x02\xd3\x03", "\x02\xb1\x01", "\x02\xab\x03", "\x02\xd4\x03",
  "\x02\xd2\x03", "\x02\x8e\x03", "\x02\x6e\x01", "\x02\x0e\x04",
  "\x02\x75\xf7", "\x02\xae\x04", "\x02\xb0\x04", "\x02\x68\x01",
  "\x02\x78\x1e", "\x02\x74\x1e", "\x02\x56\x00", "\x02\xcb\x24",
  "\x02\x7e\x1e", "\x02\x12\x04", "\x02\x4e\x05", "\x02\xb2\x01",
  "\x02\x36\xff", "\x02\x48\x05", "\x02\x76\xf7", "\x02\x7c\x1e",
  "\x02\x57\x00", "\x02\x82\x1e", "\x02\xcc\x24", "\x02\x74\x01",
  "\x02\x84\x1e", "\x02\x86\x1e", "\x02\x88\x1e", "\x02\x80\x1e",
  "\x02\x37\xff", "\x02\x77\xf7", "\x02\x58\x00", "\x02\xcd\x24",
  "\x02\x8c\x1e", "\x02\x8a\x1e", "\x02\x3d\x05", "\x02\x9e\x03",
  "\x02\x38\xff", "\x02\x78\xf7", "\x02\x59\x00", "\x02\xdd\x00",
  "\x02\xfd\xf7", "\x02\x62\x04", "\x02\xce\x24", "\x02\x76\x01",
  "\x02\x78\x01", "\x02\xff\xf7", "\x02\x8e\x1e", "\x02\xf4\x1e",
  "\x02\x2b\x04", "\x02\xf8\x04", "\x02\xf2\x1e", "\x02\xb3\x01",
  "\x02\xf6\x1e", "\x02\x45\x05", "\x02\x07\x04", "\x02\x52\x05",
  "\x02\x39\xff", "\x02\x79\xf7", "\x02\xf8\x1e", "\x02\x6a\x04",
  "\x02\x6c\x04", "\x02\x66\x04", "\x02\x68\x04", "\x02\x5a\x00",
  "\x02\x36\x05", "\x02\x79\x01", "\x02\x7d\x01", "\x02\xff\xf6",
  "\x02\xcf\x24", "\x02\x90\x1e", "\x02\x7b\x01", "\x02\x7b\x01",
  "\x02\x92\x1e", "\x02\x17\x04", "\x02\x98\x04", "\x02\xde\x04",
  "\x02\x96\x03", "\x02\x3a\x05", "\x02\xc1\x04", "\x02\x16\x04",
  "\x02\x96\x04", "\x02\xdc\x04", "\x02\x94\x1e", "\x02\x3a\xff",
  "\x02\x7a\xf7", "\x02\xb5\x01", "\x02\x61\x00", "\x02\x86\x09",
  "\x02\xe1\x00", "\x02\x06\x09", "\x02\x86\x0a", "\x02\x06\x0a",
  "\x02\x3e\x0a", "\x02\x03\x33", "\x02\xbe\x09", "\x02\x3e\x09",
  "\x02\xbe\x0a", "\x02\x5f\x05", "\x02\x70\x09", "\x02\x85\x09",
  "\x02\x1a\x31", "\x02\x03\x01", "\x02\xaf\x1e", "\x02\xd1\x04",
  "\x02\xb7\x1e", "\x02\xb1\x1e", "\x02\xb3\x1e", "\x02\xb5\x1e",
  "\x02\xce\x01", "\x02\xd0\x24", "\x02\xe2\x00", "\x02\xa5\x1e",
  "\x02\xad\x1e", "\x02\xa7\x1e", "\x02\xa9\x1e", "\x02\xab\x1e",
  "\x02\xb4\x00", "\x02\x17\x03", "\x02\x01\x03", "\x02\x01\x03",
  "\x02\x54\x09", "\x02\xcf\x02", "\x02\x41\x03", "\x02\x30\x04",
  "\x02\x01\x02", "\x02\x71\x0a", "\x02\x05\x09", "\x02\xe4\x00",
  "\x02\xd3\x04", "\x02\xdf\x01", "\x02\xa1\x1e", "\x02\xe1\x01",
  "\x02\xe6\x00", "\x02\xfd\x01", "\x02\x50\x31", "\x02\xe3\x01",
  "\x02\x15\x20", "\x02\xa4\x20", "\x02\x10\x04", "\x02\x11\x04",
  "\x02\x12\x04", "\x02\x13\x04", "\x02\x14\x04", "\x02\x15\x04",
  "\x02\x01\x04", "\x02\x16\x04", "\x02\x17\x04", "\x02\x18\x04",
  "\x02\x19\x04", "\x02\x1a\x04", "\x02\x1b\x04", "\x02\x1c\x04",
  "\x02\x1d\x04", "\x02\x1e\x04", "\x02\x1f\x04", "\x02\x20\x04",
  "\x02\x21\x04", "\x02\x22\x04", "\x02\x23\x04", "\x02\x24\x04",
  "\x02\x25\x04", "\x02\x26\x04", "\x02\x27\x04", "\x02\x28\x04",
  "\x02\x29\x04", "\x02\x2a\x04", "\x02\x2b\x04", "\x02\x2c\x04",
  "\x02\x2d\x04", "\x02\x2e\x04", "\x02\x2f\x04", "\x02\x90\x04",
  "\x02\x02\x04", "\x02\x03\x04", "\x02\x04\x04", "\x02\x05\x04",
  "\x02\x06\x04", "\x02\x07\x04", "\x02\x08\x04", "\x02\x09\x04",
  "\x02\x0a\x04", "\x02\x0b\x04", "\x02\x0c\x04", "\x02\x0e\x04",
  "\x02\xc4\xf6", "\x02\xc5\xf6", "\x02\x30\x04", "\x02\x31\x04",
  "\x02\x32\x04", "\x02\x33\x04", "\x02\x34\x04", "\x02\x35\x04",
  "\x02\x51\x04", "\x02\x36\x04", "\x02\x37\x04", "\x02\x38\x04",
  "\x02\x39\x04", "\x02\x3a\x04", "\x02\x3b\x04", "\x02\x3c\x04",
  "\x02\x3d\x04", "\x02\x3e\x04", "\x02\x3f\x04", "\x02\x40\x04",
  "\x02\x41\x04", "\x02\x42\x04", "\x02\x43\x04", "\x02\x44\x04",
  "\x02\x45\x04", "\x02\x46\x04", "\x02\x47\x04", "\x02\x48\x04",
  "\x02\x49\x04", "\x02\x4a\x04", "\x02\x4b\x04", "\x02\x4c\x04",
  "\x02\x4d\x04", "\x02\x4e\x04", "\x02\x4f\x04", "\x02\x91\x04",
  "\x02\x52\x04", "\x02\x53\x04", "\x02\x54\x04", "\x02\x55\x04",
  "\x02\x56\x04", "\x02\x57\x04", "\x02\x58\x04", "\x02\x59\x04",
  "\x02\x5a\x04", "\x02\x5b\x04", "\x02\x5c\x04", "\x02\x5e\x04",
  "\x02\x0f\x04", "\x02\x62\x04", "\x02\x72\x04", "\x02\x74\x04",
  "\x02\xc6\xf6", "\x02\x5f\x04", "\x02\x63\x04", "\x02\x73\x04",
  "\x02\x75\x04", "\x02\xc7\xf6", "\x02\xc8\xf6", "\x02\xd9\x04",
  "\x02\x0e\x20", "\x02\x0f\x20", "\x02\x0d\x20", "\x02\x6a\x06",
  "\x02\x0c\x06", "\x02\x60\x06", "\x02\x61\x06", "\x02\x62\x06",
  "\x02\x63\x06", "\x02\x64\x06", "\x02\x65\x06", "\x02\x66\x06",
  "\x02\x67\x06", "\x02\x68\x06", "\x02\x69\x06", "\x02\x1b\x06",
  "\x02\x1f\x06", "\x02\x21\x06", "\x02\x22\x06", "\x02\x23\x06",
  "\x02\x24\x06", "\x02\x25\x06", "\x02\x26\x06", "\x02\x27\x06",
  "\x02\x28\x06", "\x02\x29\x06", "\x02\x2a\x06", "\x02\x2b\x06",
  "\x02\x2c\x06", "\x02\x2d\x06", "\x02\x2e\x06", "\x02\x2f\x06",
  "\x02\x30\x06", "\x02\x31\x06", "\x02\x32\x06", "\x02\x33\x06",
  "\x02\x34\x06", "\x02\x35\x06", "\x02\x36\x06", "\x02\x37\x06",
  "\x02\x38\x06", "\x02\x39\x06", "\x02\x3a\x06", "\x02\x40\x06",
  "\x02\x41\x06", "\x02\x42\x06", "\x02\x43\x06", "\x02\x44\x06",
  "\x02\x45\x06", "\x02\x46\x06", "\x02\x48\x06", "\x02\x49\x06",
  "\x02\x4a\x06", "\x02\x4b\x06", "\x02\x4c\x06", "\x02\x4d\x06",
  "\x02\x4e\x06", "\x02\x4f\x06", "\x02\x50\x06", "\x02\x51\x06",
  "\x02\x52\x06", "\x02\x47\x06", "\x02\xa4\x06", "\x02\x7e\x06",
  "\x02\x86\x06", "\x02\x98\x06", "\x02\xaf\x06", "\x02\x79\x06",
  "\x02\x88\x06", "\x02\x91\x06", "\x02\xba\x06", "\x02\xd2\x06",
  "\x02\xd5\x06", "\x02\xaa\x20", "\x02\xbe\x05", "\x02\xc3\x05",
  "\x02\xd0\x05", "\x02\xd1\x05", "\x02\xd2\x05", "\x02\xd3\x05",
  "\x02\xd4\x05", "\x02\xd5\x05", "\x02\xd6\x05", "\x02\xd7\x05",
  "\x02\xd8\x05", "\x02\xd9\x05", "\x02\xda\x05", "\x02\xdb\x05",
  "\x02\xdc\x05", "\x02\xdd\x05", "\x02\xde\x05", "\x02\xdf\x05",
  "\x02\xe0\x05", "\x02\xe1\x05", "\x02\xe2\x05", "\x02\xe3\x05",
  "\x02\xe4\x05", "\x02\xe5\x05", "\x02\xe6\x05", "\x02\xe7\x05",
  "\x02\xe8\x05", "\x02\xe9\x05", "\x02\xea\x05", "\x02\x2a\xfb",
  "\x02\x2b\xfb", "\x02\x4b\xfb", "\x02\x1f\xfb", "\x02\xf0\x05",
  "\x02\xf1\x05", "\x02\xf2\x05", "\x02\x35\xfb", "\x02\xb4\x05",
  "\x02\xb5\x05", "\x02\xb6\x05", "\x02\xbb\x05", "\x02\xb8\x05",
  "\x02\xb7\x05", "\x02\xb0\x05", "\x02\xb2\x05", "\x02\xb1\x05",
  "\x02\xb3\x05", "\x02\xc2\x05", "\x02\xc1\x05", "\x02\xb9\x05",
  "\x02\xbc\x05", "\x02\xbd\x05", "\x02\xbf\x05", "\x02\xc0\x05",
  "\x02\xbc\x02", "\x02\x05\x21", "\x02\x13\x21", "\x02\x16\x21",
  "\x02\x2c\x20", "\x02\x2d\x20", "\x02\x2e\x20", "\x02\x0c\x20",
  "\x02\x6d\x06", "\x02\xbd\x02", "\x02\xe0\x00", "\x02\x85\x0a",
  "\x02\x05\x0a", "\x02\x42\x30", "\x02\xa3\x1e", "\x02\x90\x09",
  "\x02\x1e\x31", "\x02\x10\x09", "\x02\xd5\x04", "\x02\x90\x0a",
  "\x02\x10\x0a", "\x02\x48\x0a", "\x02\x39\x06", "\x02\xca\xfe",
  "\x02\xcb\xfe", "\x02\xcc\xfe", "\x02\x03\x02", "\x02\xc8\x09",
  "\x02\x48\x09", "\x02\xc8\x0a", "\x02\xa2\x30", "\x02\x71\xff",
  "\x02\x4f\x31", "\x02\xd0\x05", "\x02\x27\x06", "\x02\x30\xfb",
  "\x02\x8e\xfe", "\x02\x23\x06", "\x02\x84\xfe", "\x02\x25\x06",
  "\x02\x88\xfe", "\x02\xd0\x05", "\x02\x4f\xfb", "\x02\x22\x06",
  "\x02\x82\xfe", "\x02\x49\x06", "\x02\xf0\xfe", "\x02\xf3\xfe",
  "\x02\xf4\xfe", "\x02\x2e\xfb", "\x02\x2f\xfb", "\x02\x35\x21",
  "\x02\x4c\x22", "\x02\xb1\x03", "\x02\xac\x03", "\x02\x01\x01",
  "\x02\x41\xff", "\x02\x26\x00", "\x02\x06\xff", "\x02\x26\xf7",
  "\x02\xc2\x33", "\x02\x22\x31", "\x02\x24\x31", "\x02\x5a\x0e",
  "\x02\x20\x22", "\x02\x08\x30", "\x02\x3f\xfe", "\x02\x09\x30",
  "\x02\x40\xfe", "\x02\x29\x23", "\x02\x2a\x23", "\x02\x2b\x21",
  "\x02\x87\x03", "\x02\x52\x09", "\x02\x82\x09", "\x02\x02\x09",
  "\x02\x82\x0a", "\x02\x05\x01", "\x02\x00\x33", "\x02\x9c\x24",
  "\x02\x5a\x05", "\x02\xbc\x02", "\x02\xff\xf8", "\x02\x50\x22",
  "\x02\x48\x22", "\x02\x52\x22", "\x02\x45\x22", "\x02\x8e\x31",
  "\x02\x8d\x31", "\x02\x12\x23", "\x02\x9a\x1e", "\x02\xe5\x00",
  "\x02\xfb\x01", "\x02\x01\x1e", "\x02\x94\x21", "\x02\xe3\x21",
  "\x02\xe0\x21", "\x02\xe2\x21", "\x02\xe1\x21", "\x02\xd4\x21",
  "\x02\xd3\x21", "\x02\xd0\x21", "\x02\xd2\x21", "\x02\xd1\x21",
  "\x02\x93\x21", "\x02\x99\x21", "\x02\x98\x21", "\x02\xe9\x21",
  "\x02\xc5\x02", "\x02\xc2\x02", "\x02\xc3\x02", "\x02\xc4\x02",
  "\x02\xe7\xf8", "\x02\x90\x21", "\x02\xd0\x21", "\x02\xcd\x21",
  "\x02\xc6\x21", "\x02\xe6\x21", "\x02\x92\x21", "\x02\xcf\x21",
  "\x02\x9e\x27", "\x02\xc4\x21", "\x02\xe8\x21", "\x02\xe4\x21",
  "\x02\xe5\x21", "\x02\x91\x21", "\x02\x95\x21", "\x02\xa8\x21",
  "\x02\xa8\x21", "\x02\x96\x21", "\x02\xc5\x21", "\x02\x97\x21",
  "\x02\xe7\x21", "\x02\xe6\xf8", "\x02\x5e\x00", "\x02\x3e\xff",
  "\x02\x7e\x00", "\x02\x5e\xff", "\x02\x51\x02", "\x02\x52\x02",
  "\x02\x41\x30", "\x02\xa1\x30", "\x02\x67\xff", "\x02\x2a\x00",
  "\x02\x6d\x06", "\x02\x6d\x06", "\x02\x17\x22", "\x02\x0a\xff",
  "\x02\x61\xfe", "\x02\x42\x20", "\x02\xe9\xf6", "\x02\x43\x22",
  "\x02\x40\x00", "\x02\xe3\x00", "\x02\x20\xff", "\x02\x6b\xfe",
  "\x02\x50\x02", "\x02\x94\x09", "\x02\x20\x31", "\x02\x14\x09",
  "\x02\x94\x0a", "\x02\x14\x0a", "\x02\xd7\x09", "\x02\x4c\x0a",
  "\x02\xcc\x09", "\x02\x4c\x09", "\x02\xcc\x0a", "\x02\x3d\x09",
  "\x02\x61\x05", "\x02\xe2\x05", "\x02\x20\xfb", "\x02\xe2\x05",
  "\x02\x62\x00", "\x02\xac\x09", "\x02\x5c\x00", "\x02\x3c\xff",
  "\x02\x2c\x09", "\x02\xac\x0a", "\x02\x2c\x0a", "\x02\x70\x30",
  "\x02\x3f\x0e", "\x02\xd0\x30", "\x02\x7c\x00", "\x02\x5c\xff",
  "\x02\x05\x31", "\x02\xd1\x24", "\x02\x03\x1e", "\x02\x05\x1e",
  "\x02\x6c\x26", "\x02\x35\x22", "\x02\x31\x04", "\x02\x28\x06",
  "\x02\x90\xfe", "\x02\x91\xfe", "\x02\x79\x30", "\x02\x92\xfe",
  "\x02\x9f\xfc", "\x02\x08\xfc", "\x02\x6d\xfc", "\x02\xd9\x30",
  "\x02\x62\x05", "\x02\xd1\x05", "\x02\xb2\x03", "\x02\xd0\x03",
  "\x02\x31\xfb", "\x02\x31\xfb", "\x02\xd1\x05", "\x02\x4c\xfb",
  "\x02\xad\x09", "\x02\x2d\x09", "\x02\xad\x0a", "\x02\x2d\x0a",
  "\x02\x53\x02", "\x02\x73\x30", "\x02\xd3\x30", "\x02\x98\x02",
  "\x02\x02\x0a", "\x02\x31\x33", "\x02\xcf\x25", "\x02\xc6\x25",
  "\x02\xbc\x25", "\x02\xc4\x25", "\x02\xc0\x25", "\x02\x10\x30",
  "\x02\x3b\xfe", "\x02\x11\x30", "\x02\x3c\xfe", "\x02\xe3\x25",
  "\x02\xe2\x25", "\x02\xac\x25", "\x02\xba\x25", "\x02\xb6\x25",
  "\x02\xaa\x25", "\x02\x3b\x26", "\x02\xa0\x25", "\x02\x05\x26",
  "\x02\xe4\x25", "\x02\xe5\x25", "\x02\xb4\x25", "\x02\xb2\x25",
  "\x02\x23\x24", "\x02\x07\x1e", "\x02\x88\x25", "\x02\x42\xff",
  "\x02\x1a\x0e", "\x02\x7c\x30", "\x02\xdc\x30", "\x02\x9d\x24",
  "\x02\xc3\x33", "\x02\xf4\xf8", "\x02\x7b\x00", "\x02\xf3\xf8",
  "\x02\xf2\xf8", "\x02\x5b\xff", "\x02\x5b\xfe", "\x02\xf1\xf8",
  "\x02\x37\xfe", "\x02\x7d\x00", "\x02\xfe\xf8", "\x02\xfd\xf8",
  "\x02\x5d\xff", "\x02\x5c\xfe", "\x02\xfc\xf8", "\x02\x38\xfe",
  "\x02\x5b\x00", "\x02\xf0\xf8", "\x02\xef\xf8", "\x02\x3b\xff",
  "\x02\xee\xf8", "\x02\x5d\x00", "\x02\xfb\xf8", "\x02\xfa\xf8",
  "\x02\x3d\xff", "\x02\xf9\xf8", "\x02\xd8\x02", "\x02\x2e\x03",
  "\x02\x06\x03", "\x02\x2f\x03", "\x02\x11\x03", "\x02\x61\x03",
  "\x02\x2a\x03", "\x02\x3a\x03", "\x02\xa6\x00", "\x02\x80\x01",
  "\x02\xea\xf6", "\x02\x83\x01", "\x02\x76\x30", "\x02\xd6\x30",
  "\x02\x22\x20", "\x02\xd8\x25", "\x02\x19\x22", "\x02\xce\x25",
  "\x02\x63\x00", "\x02\x6e\x05", "\x02\x9a\x09", "\x02\x07\x01",
  "\x02\x1a\x09", "\x02\x9a\x0a", "\x02\x1a\x0a", "\x02\x88\x33",
  "\x02\x81\x09", "\x02\x10\x03", "\x02\x01\x09", "\x02\x81\x0a",
  "\x02\xea\x21", "\x02\x05\x21", "\x02\xc7\x02", "\x02\x2c\x03",
  "\x02\x0c\x03", "\x02\xb5\x21", "\x02\x18\x31", "\x02\x0d\x01",
  "\x02\xe7\x00", "\x02\x09\x1e", "\x02\xd2\x24", "\x02\x09\x01",
  "\x02\x55\x02", "\x02\x0b\x01", "\x02\x0b\x01", "\x02\xc5\x33",
  "\x02\xb8\x00", "\x02\x27\x03", "\x02\xa2\x00", "\x02\x03\x21",
  "\x02\xdf\xf6", "\x02\xe0\xff", "\x02\xa2\xf7", "\x02\xe0\xf6",
  "\x02\x79\x05", "\x02\x9b\x09", "\x02\x1b\x09", "\x02\x9b\x0a",
  "\x02\x1b\x0a", "\x02\x14\x31", "\x02\xbd\x04", "\x02\x13\x27",
  "\x02\x47\x04", "\x02\xbf\x04", "\x02\xb7\x04", "\x02\xf5\x04",
  "\x02\x73\x05", "\x02\xcc\x04", "\x02\xb9\x04", "\x02\xc7\x03",
  "\x02\x77\x32", "\x02\x17\x32", "\x02\x69\x32", "\x02\x4a\x31",
  "\x02\x09\x32", "\x02\x0a\x0e", "\x02\x08\x0e", "\x02\x09\x0e",
  "\x02\x0c\x0e", "\x02\x88\x01", "\x02\x76\x32", "\x02\x16\x32",
  "\x02\x68\x32", "\x02\x48\x31", "\x02\x08\x32", "\x02\x1c\x32",
  "\x02\xcb\x25", "\x02\x97\x22", "\x02\x99\x22", "\x02\x95\x22",
  "\x02\x36\x30", "\x02\xd0\x25", "\x02\xd1\x25", "\x02\xc6\x02",
  "\x02\x2d\x03", "\x02\x02\x03", "\x02\x27\x23", "\x02\xc2\x01",
  "\x02\xc0\x01", "\x02\xc1\x01", "\x02\xc3\x01", "\x02\x63\x26",
  "\x02\x63\x26", "\x02\x67\x26", "\x02\xa4\x33", "\x02\x43\xff",
  "\x02\xa0\x33", "\x02\x81\x05", "\x02\x3a\x00", "\x02\xa1\x20",
  "\x02\x1a\xff", "\x02\xa1\x20", "\x02\x55\xfe", "\x02\xd1\x02",
  "\x02\xd0\x02", "\x02\x2c\x00", "\x02\x13\x03", "\x02\x15\x03",
  "\x02\xc3\xf6", "\x02\x0c\x06", "\x02\x5d\x05", "\x02\xe1\xf6",
  "\x02\x0c\xff", "\x02\x14\x03", "\x02\xbd\x02", "\x02\x50\xfe",
  "\x02\xe2\xf6", "\x02\x12\x03", "\x02\xbb\x02", "\x02\x3c\x26",
  "\x02\x45\x22", "\x02\x2e\x22", "\x02\x03\x23", "\x02\x06\x00",
  "\x02\x07\x00", "\x02\x08\x00", "\x02\x18\x00", "\x02\x0d\x00",
  "\x02\x11\x00", "\x02\x12\x00", "\x02\x13\x00", "\x02\x14\x00",
  "\x02\x7f\x00", "\x02\x10\x00", "\x02\x19\x00", "\x02\x05\x00",
  "\x02\x04\x00", "\x02\x1b\x00", "\x02\x17\x00", "\x02\x03\x00",
  "\x02\x0c\x00", "\x02\x1c\x00", "\x02\x1d\x00", "\x02\x09\x00",
  "\x02\x0a\x00", "\x02\x15\x00", "\x02\x1e\x00", "\x02\x0f\x00",
  "\x02\x0e\x00", "\x02\x02\x00", "\x02\x01\x00", "\x02\x1a\x00",
  "\x02\x16\x00", "\x02\x1f\x00", "\x02\x0b\x00", "\x02\xa9\x00",
  "\x02\xe9\xf8", "\x02\xd9\xf6", "\x02\x0c\x30", "\x02\x62\xff",
  "\x02\x41\xfe", "\x02\x0d\x30", "\x02\x63\xff", "\x02\x42\xfe",
  "\x02\x7f\x33", "\x02\xc7\x33", "\x02\xc6\x33", "\x02\x9e\x24",
  "\x02\xa2\x20", "\x02\x97\x02", "\x02\xcf\x22", "\x02\xce\x22",
  "\x02\xa4\x00", "\x02\xd1\xf6", "\x02\xd2\xf6", "\x02\xd4\xf6",
  "\x02\xd5\xf6", "\x02\x64\x00", "\x02\x64\x05", "\x02\xa6\x09",
  "\x02\x36\x06", "\x02\x26\x09", "\x02\xbe\xfe", "\x02\xbf\xfe",
  "\x02\xc0\xfe", "\x02\xbc\x05", "\x02\xbc\x05", "\x02\x20\x20",
  "\x02\x21\x20", "\x02\xa6\x0a", "\x02\x26\x0a", "\x02\x60\x30",
  "\x02\xc0\x30", "\x02\x2f\x06", "\x02\xd3\x05", "\x02\x33\xfb",
  "\x02\x33\xfb", "\x04\xd3\x05\xb2\x05", "\x04\xd3\x05\xb2\x05",
  "\x04\xd3\x05\xb1\x05", "\x04\xd3\x05\xb1\x05", "\x02\xd3\x05",
  "\x04\xd3\x05\xb4\x05", "\x04\xd3\x05\xb4\x05", "\x04\xd3\x05\xb9\x05",
  "\x04\xd3\x05\xb9\x05", "\x04\xd3\x05\xb7\x05", "\x04\xd3\x05\xb7\x05",
  "\x04\xd3\x05\xb8\x05", "\x04\xd3\x05\xb8\x05", "\x04\xd3\x05\xbb\x05",
  "\x04\xd3\x05\xbb\x05", "\x04\xd3\x05\xb6\x05", "\x04\xd3\x05\xb6\x05",
  "\x04\xd3\x05\xb0\x05", "\x04\xd3\x05\xb0\x05", "\x04\xd3\x05\xb5\x05",
  "\x04\xd3\x05\xb5\x05", "\x02\xaa\xfe", "\x02\x4f\x06", "\x02\x4f\x06",
  "\x02\x4c\x06", "\x02\x4c\x06", "\x02\x64\x09", "\x02\xa7\x05",
  "\x02\xa7\x05", "\x02\x85\x04", "\x02\xd3\xf6", "\x02\x0a\x30",
  "\x02\x3d\xfe", "\x02\x0b\x30", "\x02\x3e\xfe", "\x02\x2b\x03",
  "\x02\xd4\x21", "\x02\xd2\x21", "\x02\x65\x09", "\x02\xd6\xf6",
  "\x02\x0f\x03", "\x02\x2c\x22", "\x02\x17\x20", "\x02\x33\x03",
  "\x02\x3f\x03", "\x02\xba\x02", "\x02\x16\x20", "\x02\x0e\x03",
  "\x02\x09\x31", "\x02\xc8\x33", "\x02\x0f\x01", "\x02\x11\x1e",
  "\x02\xd3\x24", "\x02\x13\x1e", "\x02\x11\x01", "\x02\xa1\x09",
  "\x02\x21\x09", "\x02\xa1\x0a", "\x02\x21\x0a", "\x02\x88\x06",
  "\x02\x89\xfb", "\x02\x5c\x09", "\x02\xa2\x09", "\x02\x22\x09",
  "\x02\xa2\x0a", "\x02\x22\x0a", "\x02\x0b\x1e", "\x02\x0d\x1e",
  "\x02\x6b\x06", "\x02\x6b\x06", "\x02\x34\x04", "\x02\xb0\x00",
  "\x02\xad\x05", "\x02\x67\x30", "\x02\xef\x03", "\x02\xc7\x30",
  "\x02\x2b\x23", "\x02\x26\x23", "\x02\xb4\x03", "\x02\x8d\x01",
  "\x02\xf8\x09", "\x02\xa4\x02", "\x02\xa7\x09", "\x02\x27\x09",
  "\x02\xa7\x0a", "\x02\x27\x0a", "\x02\x57\x02", "\x02\x85\x03",
  "\x02\x44\x03", "\x02\x66\x26", "\x02\x62\x26", "\x02\xa8\x00",
  "\x02\xd7\xf6", "\x02\x24\x03", "\x02\x08\x03", "\x02\xd8\xf6",
  "\x02\x85\x03", "\x02\x62\x30", "\x02\xc2\x30", "\x02\x03\x30",
  "\x02\xf7\x00", "\x02\x23\x22", "\x02\x15\x22", "\x02\x52\x04",
  "\x02\x93\x25", "\x02\x0f\x1e", "\x02\x97\x33", "\x02\x11\x01",
  "\x02\x44\xff", "\x02\x84\x25", "\x02\x0e\x0e", "\x02\x14\x0e",
  "\x02\x69\x30", "\x02\xc9\x30", "\x02\x24\x00", "\x02\xe3\xf6",
  "\x02\x04\xff", "\x02\x24\xf7", "\x02\x69\xfe", "\x02\xe4\xf6",
  "\x02\xab\x20", "\x02\x26\x33", "\x02\xd9\x02", "\x02\x07\x03",
  "\x02\x23\x03", "\x02\x23\x03", "\x02\xfb\x30", "\x02\x31\x01",
  "\x02\xbe\xf6", "\x02\x84\x02", "\x02\xc5\x22", "\x02\xcc\x25",
  "\x02\x1f\xfb", "\x02\x1f\xfb", "\x02\x1e\x03", "\x02\xd5\x02",
  "\x02\x9f\x24", "\x02\xeb\xf6", "\x02\x56\x02", "\x02\x8c\x01",
  "\x02\x65\x30", "\x02\xc5\x30", "\x02\xf3\x01", "\x02\xa3\x02",
  "\x02\xc6\x01", "\x02\xa5\x02", "\x02\xe1\x04", "\x02\x55\x04",
  "\x02\x5f\x04", "\x02\x65\x00", "\x02\xe9\x00", "\x02\x41\x26",
  "\x02\x8f\x09", "\x02\x1c\x31", "\x02\x15\x01", "\x02\x0d\x09",
  "\x02\x8d\x0a", "\x02\x45\x09", "\x02\xc5\x0a", "\x02\x1b\x01",
  "\x02\x1d\x1e", "\x02\x65\x05", "\x02\x87\x05", "\x02\xd4\x24",
  "\x02\xea\x00", "\x02\xbf\x1e", "\x02\x19\x1e", "\x02\xc7\x1e",
  "\x02\xc1\x1e", "\x02\xc3\x1e", "\x02\xc5\x1e", "\x02\x54\x04",
  "\x02\x05\x02", "\x02\x0f\x09", "\x02\xeb\x00", "\x02\x17\x01",
  "\x02\x17\x01", "\x02\xb9\x1e", "\x02\x0f\x0a", "\x02\x47\x0a",
  "\x02\x44\x04", "\x02\xe8\x00", "\x02\x8f\x0a", "\x02\x67\x05",
  "\x02\x1d\x31", "\x02\x48\x30", "\x02\xbb\x1e", "\x02\x1f\x31",
  "\x02\x38\x00", "\x02\x68\x06", "\x02\xee\x09", "\x02\x67\x24",
  "\x02\x91\x27", "\x02\x6e\x09", "\x02\x71\x24", "\x02\x85\x24",
  "\x02\x99\x24", "\x02\xee\x0a", "\x02\x6e\x0a", "\x02\x68\x06",
  "\x02\x28\x30", "\x02\x6b\x26", "\x02\x27\x32", "\x02\x88\x20",
  "\x02\x18\xff", "\x02\x38\xf7", "\x02\x7b\x24", "\x02\x8f\x24",
  "\x02\xf8\x06", "\x02\x77\x21", "\x02\x78\x20", "\x02\x58\x0e",
  "\x02\x07\x02", "\x02\x65\x04", "\x02\xa8\x30", "\x02\x74\xff",
  "\x02\x74\x0a", "\x02\x54\x31", "\x02\x3b\x04", "\x02\x08\x22",
  "\x02\x6a\x24", "\x02\x7e\x24", "\x02\x92\x24", "\x02\x7a\x21",
  "\x02\x26\x20", "\x02\xee\x22", "\x02\x13\x01", "\x02\x17\x1e",
  "\x02\x15\x1e", "\x02\x3c\x04", "\x02\x14\x20", "\x02\x31\xfe",
  "\x02\x45\xff", "\x02\x5b\x05", "\x02\x05\x22", "\x02\x23\x31",
  "\x02\x3d\x04", "\x02\x13\x20", "\x02\x32\xfe", "\x02\xa3\x04",
  "\x02\x4b\x01", "\x02\x25\x31", "\x02\xa5\x04", "\x02\xc8\x04",
  "\x02\x02\x20", "\x02\x19\x01", "\x02\x53\x31", "\x02\x5b\x02",
  "\x02\x9a\x02", "\x02\x5c\x02", "\x02\x5e\x02", "\x02\x5d\x02",
  "\x02\xa0\x24", "\x02\xb5\x03", "\x02\xad\x03", "\x02\x3d\x00",
  "\x02\x1d\xff", "\x02\x66\xfe", "\x02\x7c\x20", "\x02\x61\x22",
  "\x02\x26\x31", "\x02\x40\x04", "\x02\x58\x02", "\x02\x4d\x04",
  "\x02\x41\x04", "\x02\xab\x04", "\x02\x83\x02", "\x02\x86\x02",
  "\x02\x0e\x09", "\x02\x46\x09", "\x02\xaa\x01", "\x02\x85\x02",
  "\x02\x47\x30", "\x02\xa7\x30", "\x02\x6a\xff", "\x02\x2e\x21",
  "\x02\xec\xf6", "\x02\xb7\x03", "\x02\x68\x05", "\x02\xae\x03",
  "\x02\xf0\x00", "\x02\xbd\x1e", "\x02\x1b\x1e", "\x02\x91\x05",
  "\x02\x91\x05", "\x02\x91\x05", "\x02\x91\x05", "\x02\xdd\x01",
  "\x02\x61\x31", "\x02\xac\x20", "\x02\xc7\x09", "\x02\x47\x09",
  "\x02\xc7\x0a", "\x02\x21\x00", "\x02\x5c\x05", "\x02\x3c\x20",
  "\x02\xa1\x00", "\x02\xa1\xf7", "\x02\x01\xff", "\x02\x21\xf7",
  "\x02\x03\x22", "\x02\x92\x02", "\x02\xef\x01", "\x02\x93\x02",
  "\x02\xb9\x01", "\x02\xba\x01", "\x02\x66\x00", "\x02\x5e\x09",
  "\x02\x5e\x0a", "\x02\x09\x21", "\x02\x4e\x06", "\x02\x4e\x06",
  "\x02\x4b\x06", "\x02\x08\x31", "\x02\xd5\x24", "\x02\x1f\x1e",
  "\x02\x41\x06", "\x02\x86\x05", "\x02\xd2\xfe", "\x02\xd3\xfe",
  "\x02\xd4\xfe", "\x02\xe5\x03", "\x02\x40\x26", "\x02\x00\xfb",
  "\x02\x03\xfb", "\x02\x04\xfb", "\x02\x01\xfb", "\x02\x6e\x24",
  "\x02\x82\x24", "\x02\x96\x24", "\x02\x12\x20", "\x02\xa0\x25",
  "\x02\xac\x25", "\x02\xda\x05", "\x02\x3a\xfb", "\x02\x3a\xfb",
  "\x02\xda\x05", "\x04\xda\x05\xb8\x05", "\x04\xda\x05\xb8\x05",
  "\x04\xda\x05\xb0\x05", "\x04\xda\x05\xb0\x05", "\x02\xdd\x05",
  "\x02\xdd\x05", "\x02\xdf\x05", "\x02\xdf\x05", "\x02\xe3\x05",
  "\x02\xe3\x05", "\x02\xe5\x05", "\x02\xe5\x05", "\x02\xc9\x02",
  "\x02\xc9\x25", "\x02\x73\x04", "\x02\x35\x00", "\x02\x65\x06",
  "\x02\xeb\x09", "\x02\x64\x24", "\x02\x8e\x27", "\x02\x6b\x09",
  "\x02\x5d\x21", "\x02\xeb\x0a", "\x02\x6b\x0a", "\x02\x65\x06",
  "\x02\x25\x30", "\x02\x24\x32", "\x02\x85\x20", "\x02\x15\xff",
  "\x02\x35\xf7", "\x02\x78\x24", "\x02\x8c\x24", "\x02\xf5\x06",
  "\x02\x74\x21", "\x02\x75\x20", "\x02\x55\x0e", "\x02\x02\xfb",
  "\x02\x92\x01", "\x02\x46\xff", "\x02\x99\x33", "\x02\x1f\x0e",
  "\x02\x1d\x0e", "\x02\x4f\x0e", "\x02\x00\x22", "\x02\x34\x00",
  "\x02\x64\x06", "\x02\xea\x09", "\x02\x63\x24", "\x02\x8d\x27",
  "\x02\x6a\x09", "\x02\xea\x0a", "\x02\x6a\x0a", "\x02\x64\x06",
  "\x02\x24\x30", "\x02\x23\x32", "\x02\x84\x20", "\x02\x14\xff",
  "\x02\xf7\x09", "\x02\x34\xf7", "\x02\x77\x24", "\x02\x8b\x24",
  "\x02\xf4\x06", "\x02\x73\x21", "\x02\x74\x20", "\x02\x6d\x24",
  "\x02\x81\x24", "\x02\x95\x24", "\x02\x54\x0e", "\x02\xcb\x02",
  "\x02\xa1\x24", "\x02\x44\x20", "\x02\xa3\x20", "\x02\x67\x00",
  "\x02\x97\x09", "\x02\xf5\x01", "\x02\x17\x09", "\x02\xaf\x06",
  "\x02\x93\xfb", "\x02\x94\xfb", "\x02\x95\xfb", "\x02\x97\x0a",
  "\x02\x17\x0a", "\x02\x4c\x30", "\x02\xac\x30", "\x02\xb3\x03",
  "\x02\x63\x02", "\x02\xe0\x02", "\x02\xeb\x03", "\x02\x0d\x31",
  "\x02\x1f\x01", "\x02\xe7\x01", "\x02\x23\x01", "\x02\xd6\x24",
  "\x02\x1d\x01", "\x02\x23\x01", "\x02\x21\x01", "\x02\x21\x01",
  "\x02\x33\x04", "\x02\x52\x30", "\x02\xb2\x30", "\x02\x51\x22",
  "\x02\x9c\x05", "\x02\xf3\x05", "\x02\x9d\x05", "\x02\xdf\x00",
  "\x02\x9e\x05", "\x02\xf4\x05", "\x02\x13\x30", "\x02\x98\x09",
  "\x02\x72\x05", "\x02\x18\x09", "\x02\x98\x0a", "\x02\x18\x0a",
  "\x02\x3a\x06", "\x02\xce\xfe", "\x02\xcf\xfe", "\x02\xd0\xfe",
  "\x02\x95\x04", "\x02\x93\x04", "\x02\x91\x04", "\x02\x5a\x09",
  "\x02\x5a\x0a", "\x02\x60\x02", "\x02\x93\x33", "\x02\x4e\x30",
  "\x02\xae\x30", "\x02\x63\x05", "\x02\xd2\x05", "\x02\x32\xfb",
  "\x02\x32\xfb", "\x02\xd2\x05", "\x02\x53\x04", "\x02\xbe\x01",
  "\x02\x94\x02", "\x02\x96\x02", "\x02\xc0\x02", "\x02\x95\x02",
  "\x02\xc1\x02", "\x02\xe4\x02", "\x02\xa1\x02", "\x02\xa2\x02",
  "\x02\x21\x1e", "\x02\x47\xff", "\x02\x54\x30", "\x02\xb4\x30",
  "\x02\xa2\x24", "\x02\xac\x33", "\x02\x07\x22", "\x02\x60\x00",
  "\x02\x16\x03", "\x02\x00\x03", "\x02\x00\x03", "\x02\x53\x09",
  "\x02\xce\x02", "\x02\x40\xff", "\x02\x40\x03", "\x02\x3e\x00",
  "\x02\x65\x22", "\x02\xdb\x22", "\x02\x1e\xff", "\x02\x73\x22",
  "\x02\x77\x22", "\x02\x67\x22", "\x02\x65\xfe", "\x02\x61\x02",
  "\x02\xe5\x01", "\x02\x50\x30", "\x02\xab\x00", "\x02\xbb\x00",
  "\x02\x39\x20", "\x02\x3a\x20", "\x02\xb0\x30", "\x02\x18\x33",
  "\x02\xc9\x33", "\x02\x68\x00", "\x02\xa9\x04", "\x02\xc1\x06",
  "\x02\xb9\x09", "\x02\xb3\x04", "\x02\x39\x09", "\x02\xb9\x0a",
  "\x02\x39\x0a", "\x02\x2d\x06", "\x02\xa2\xfe", "\x02\xa3\xfe",
  "\x02\x6f\x30", "\x02\xa4\xfe", "\x02\x2a\x33", "\x02\xcf\x30",
  "\x02\x8a\xff", "\x02\x4d\x0a", "\x02\x21\x06", "\x04\x21\x06\x4f\x06",
  "\x04\x21\x06\x4c\x06", "\x04\x21\x06\x4e\x06", "\x04\x21\x06\x4b\x06",
  "\x02\x21\x06", "\x04\x21\x06\x50\x06", "\x04\x21\x06\x4d\x06",
  "\x04\x21\x06\x52\x06", "\x02\x64\x31", "\x02\x4a\x04", "\x02\xbc\x21",
  "\x02\xc0\x21", "\x02\xca\x33", "\x02\xb2\x05", "\x02\xb2\x05",
  "\x02\xb2\x05", "\x02\xb2\x05", "\x02\xb2\x05", "\x02\xb2\x05",
  "\x02\xb2\x05", "\x02\xb2\x05", "\x02\xb3\x05", "\x02\xb3\x05",
  "\x02\xb3\x05", "\x02\xb3\x05", "\x02\xb3\x05", "\x02\xb3\x05",
  "\x02\xb3\x05", "\x02\xb3\x05", "\x02\xb1\x05", "\x02\xb1\x05",
  "\x02\xb1\x05", "\x02\xb1\x05", "\x02\xb1\x05", "\x02\xb1\x05",
  "\x02\xb1\x05", "\x02\xb1\x05", "\x02\x27\x01", "\x02\x0f\x31",
  "\x02\x2b\x1e", "\x02\x29\x1e", "\x02\xd7\x24", "\x02\x25\x01",
  "\x02\x27\x1e", "\x02\x23\x1e", "\x02\x25\x1e", "\x02\xd4\x05",
  "\x02\x65\x26", "\x02\x65\x26", "\x02\x61\x26", "\x02\x34\xfb",
  "\x02\x34\xfb", "\x02\xc1\x06", "\x02\x47\x06", "\x02\xd4\x05",
  "\x02\xa7\xfb", "\x02\xea\xfe", "\x02\xea\xfe", "\x02\xa5\xfb",
  "\x02\xa4\xfb", "\x02\xa8\xfb", "\x02\xeb\xfe", "\x02\x78\x30",
  "\x02\xa9\xfb", "\x02\xec\xfe", "\x02\x7b\x33", "\x02\xd8\x30",
  "\x02\x8d\xff", "\x02\x36\x33", "\x02\x67\x02", "\x02\x39\x33",
  "\x02\xd7\x05", "\x02\xd7\x05", "\x02\x66\x02", "\x02\xb1\x02",
  "\x02\x7b\x32", "\x02\x1b\x32", "\x02\x6d\x32", "\x02\x4e\x31",
  "\x02\x0d\x32", "\x02\x72\x30", "\x02\xd2\x30", "\x02\x8b\xff",
  "\x02\xb4\x05", "\x02\xb4\x05", "\x02\xb4\x05", "\x02\xb4\x05",
  "\x02\xb4\x05", "\x02\xb4\x05", "\x02\xb4\x05", "\x02\xb4\x05",
  "\x02\x96\x1e", "\x02\x48\xff", "\x02\x70\x05", "\x02\x2b\x0e",
  "\x02\x7b\x30", "\x02\xdb\x30", "\x02\x8e\xff", "\x02\xb9\x05",
  "\x02\xb9\x05", "\x02\xb9\x05", "\x02\xb9\x05", "\x02\xb9\x05",
  "\x02\xb9\x05", "\x02\xb9\x05", "\x02\xb9\x05", "\x02\x2e\x0e",
  "\x02\x09\x03", "\x02\x09\x03", "\x02\x21\x03", "\x02\x22\x03",
  "\x02\x42\x33", "\x02\xe9\x03", "\x02\x15\x20", "\x02\x1b\x03",
  "\x02\x68\x26", "\x02\x02\x23", "\x02\xa3\x24", "\x02\xb0\x02",
  "\x02\x65\x02", "\x02\x75\x30", "\x02\x33\x33", "\x02\xd5\x30",
  "\x02\x8c\xff", "\x02\xdd\x02", "\x02\x0b\x03", "\x02\x95\x01",
  "\x02\x2d\x00", "\x02\xe5\xf6", "\x02\x0d\xff", "\x02\x63\xfe",
  "\x02\xe6\xf6", "\x02\x10\x20", "\x02\x69\x00", "\x02\xed\x00",
  "\x02\x4f\x04", "\x02\x87\x09", "\x02\x27\x31", "\x02\x2d\x01",
  "\x02\xd0\x01", "\x02\xd8\x24", "\x02\xee\x00", "\x02\x56\x04",
  "\x02\x09\x02", "\x02\x8f\x32", "\x02\x8b\x32", "\x02\x3f\x32",
  "\x02\x3a\x32", "\x02\xa5\x32", "\x02\x06\x30", "\x02\x01\x30",
  "\x02\x64\xff", "\x02\x37\x32", "\x02\xa3\x32", "\x02\x2f\x32",
  "\x02\x3d\x32", "\x02\x9d\x32", "\x02\x40\x32", "\x02\x96\x32",
  "\x02\x36\x32", "\x02\x2b\x32", "\x02\x32\x32", "\x02\xa4\x32",
  "\x02\x05\x30", "\x02\x98\x32", "\x02\x38\x32", "\x02\xa7\x32",
  "\x02\xa6\x32", "\x02\xa9\x32", "\x02\x2e\x32", "\x02\x2a\x32",
  "\x02\x34\x32", "\x02\x02\x30", "\x02\x9e\x32", "\x02\x43\x32",
  "\x02\x39\x32", "\x02\x3e\x32", "\x02\xa8\x32", "\x02\x99\x32",
  "\x02\x42\x32", "\x02\x33\x32", "\x02\x00\x30", "\x02\x35\x32",
  "\x02\x31\x32", "\x02\x3b\x32", "\x02\x30\x32", "\x02\x3c\x32",
  "\x02\x2c\x32", "\x02\x2d\x32", "\x02\x07\x30", "\x02\x8e\x32",
  "\x02\x8a\x32", "\x02\x94\x32", "\x02\x90\x32", "\x02\x8c\x32",
  "\x02\x8d\x32", "\x02\x07\x09", "\x02\xef\x00", "\x02\x2f\x1e",
  "\x02\xe5\x04", "\x02\xcb\x1e", "\x02\xd7\x04", "\x02\x35\x04",
  "\x02\x75\x32", "\x02\x15\x32", "\x02\x67\x32", "\x02\x47\x31",
  "\x02\x07\x32", "\x02\xec\x00", "\x02\x87\x0a", "\x02\x07\x0a",
  "\x02\x44\x30", "\x02\xc9\x1e", "\x02\x88\x09", "\x02\x38\x04",
  "\x02\x08\x09", "\x02\x88\x0a", "\x02\x08\x0a", "\x02\x40\x0a",
  "\x02\x0b\x02", "\x02\x39\x04", "\x02\xc0\x09", "\x02\x40\x09",
  "\x02\xc0\x0a", "\x02\x33\x01", "\x02\xa4\x30", "\x02\x72\xff",
  "\x02\x63\x31", "\x02\xdc\x02", "\x02\xac\x05", "\x02\x2b\x01",
  "\x02\xe3\x04", "\x02\x53\x22", "\x02\x3f\x0a", "\x02\x49\xff",
  "\x02\x06\x22", "\x02\x1e\x22", "\x02\x6b\x05", "\x02\x2b\x22",
  "\x02\x21\x23", "\x02\x21\x23", "\x02\xf5\xf8", "\x02\x20\x23",
  "\x02\x20\x23", "\x02\x29\x22", "\x02\x05\x33", "\x02\xd8\x25",
  "\x02\xd9\x25", "\x02\x3b\x26", "\x02\x51\x04", "\x02\x2f\x01",
  "\x02\xb9\x03", "\x02\xca\x03", "\x02\x90\x03", "\x02\x69\x02",
  "\x02\xaf\x03", "\x02\xa4\x24", "\x02\x72\x0a", "\x02\x43\x30",
  "\x02\xa3\x30", "\x02\x68\xff", "\x02\xfa\x09", "\x02\x68\x02",
  "\x02\xed\xf6", "\x02\x9d\x30", "\x02\xfd\x30", "\x02\x29\x01",
  "\x02\x2d\x1e", "\x02\x29\x31", "\x02\x4e\x04", "\x02\xbf\x09",
  "\x02\x3f\x09", "\x02\xbf\x0a", "\x02\x75\x04", "\x02\x77\x04",
  "\x02\x6a\x00", "\x02\x71\x05", "\x02\x9c\x09", "\x02\x1c\x09",
  "\x02\x9c\x0a", "\x02\x1c\x0a", "\x02\x10\x31", "\x02\xf0\x01",
  "\x02\xd9\x24", "\x02\x35\x01", "\x02\x9d\x02", "\x02\x5f\x02",
  "\x02\x58\x04", "\x02\x2c\x06", "\x02\x9e\xfe", "\x02\x9f\xfe",
  "\x02\xa0\xfe", "\x02\x98\x06", "\x02\x8b\xfb", "\x02\x9d\x09",
  "\x02\x1d\x09", "\x02\x9d\x0a", "\x02\x1d\x0a", "\x02\x7b\x05",
  "\x02\x04\x30", "\x02\x4a\xff", "\x02\xa5\x24", "\x02\xb2\x02",
  "\x02\x6b\x00", "\x02\xa1\x04", "\x02\x95\x09", "\x02\x31\x1e",
  "\x02\x3a\x04", "\x02\x9b\x04", "\x02\x15\x09", "\x02\xdb\x05",
  "\x02\x43\x06", "\x02\x3b\xfb", "\x02\x3b\xfb", "\x02\xda\xfe",
  "\x02\xdb\x05", "\x02\xdb\xfe", "\x02\xdc\xfe", "\x02\x4d\xfb",
  "\x02\x95\x0a", "\x02\x15\x0a", "\x02\x4b\x30", "\x02\xc4\x04",
  "\x02\xab\x30", "\x02\x76\xff", "\x02\xba\x03", "\x02\xf0\x03",
  "\x02\x71\x31", "\x02\x84\x31", "\x02\x78\x31", "\x02\x79\x31",
  "\x02\x0d\x33", "\x02\x40\x06", "\x02\x40\x06", "\x02\xf5\x30",
  "\x02\x84\x33", "\x02\x50\x06", "\x02\x4d\x06", "\x02\x9f\x04",
  "\x02\x70\xff", "\x02\x9d\x04", "\x02\x0e\x31", "\x02\x89\x33",
  "\x02\xe9\x01", "\x02\x37\x01", "\x02\xda\x24", "\x02\x37\x01",
  "\x02\x33\x1e", "\x02\x84\x05", "\x02\x51\x30", "\x02\xb1\x30",
  "\x02\x79\xff", "\x02\x6f\x05", "\x02\xf6\x30", "\x02\x38\x01",
  "\x02\x96\x09", "\x02\x45\x04", "\x02\x16\x09", "\x02\x96\x0a",
  "\x02\x16\x0a", "\x02\x2e\x06", "\x02\xa6\xfe", "\x02\xa7\xfe",
  "\x02\xa8\xfe", "\x02\xe7\x03", "\x02\x59\x09", "\x02\x59\x0a",
  "\x02\x78\x32", "\x02\x18\x32", "\x02\x6a\x32", "\x02\x4b\x31",
  "\x02\x0a\x32", "\x02\x02\x0e", "\x02\x05\x0e", "\x02\x03\x0e",
  "\x02\x04\x0e", "\x02\x5b\x0e", "\x02\x99\x01", "\x02\x06\x0e",
  "\x02\x91\x33", "\x02\x4d\x30", "\x02\xad\x30", "\x02\x77\xff",
  "\x02\x15\x33", "\x02\x16\x33", "\x02\x14\x33", "\x02\x6e\x32",
  "\x02\x0e\x32", "\x02\x60\x32", "\x02\x31\x31", "\x02\x00\x32",
  "\x02\x33\x31", "\x02\x5c\x04", "\x02\x35\x1e", "\x02\x98\x33",
  "\x02\xa6\x33", "\x02\x4b\xff", "\x02\xa2\x33", "\x02\x53\x30",
  "\x02\xc0\x33", "\x02\x01\x0e", "\x02\xb3\x30", "\x02\x7a\xff",
  "\x02\x1e\x33", "\x02\x81\x04", "\x02\x7f\x32", "\x02\x43\x03",
  "\x02\xa6\x24", "\x02\xaa\x33", "\x02\x6f\x04", "\x02\xcf\x33",
  "\x02\x9e\x02", "\x02\x4f\x30", "\x02\xaf\x30", "\x02\x78\xff",
  "\x02\xb8\x33", "\x02\xbe\x33", "\x02\x6c\x00", "\x02\xb2\x09",
  "\x02\x3a\x01", "\x02\x32\x09", "\x02\xb2\x0a", "\x02\x32\x0a",
  "\x02\x45\x0e", "\x02\xfc\xfe", "\x02\xf8\xfe", "\x02\xf7\xfe",
  "\x02\xfa\xfe", "\x02\xf9\xfe", "\x02\xfb\xfe", "\x02\xf6\xfe",
  "\x02\xf5\xfe", "\x02\x44\x06", "\x02\xbb\x03", "\x02\x9b\x01",
  "\x02\xdc\x05", "\x02\x3c\xfb", "\x02\x3c\xfb", "\x02\xdc\x05",
  "\x04\xdc\x05\xb9\x05", "\x06\xdc\x05\xb9\x05\xbc\x05",
  "\x06\xdc\x05\xb9\x05\xbc\x05", "\x04\xdc\x05\xb9\x05", "\x02\xde\xfe",
  "\x02\xca\xfc", "\x02\xdf\xfe", "\x02\xc9\xfc", "\x02\xcb\xfc",
  "\x02\xf2\xfd", "\x02\xe0\xfe", "\x02\x88\xfd", "\x02\xcc\xfc",
  "\x06\xdf\xfe\xe4\xfe\xa0\xfe", "\x06\xdf\xfe\xe4\xfe\xa8\xfe",
  "\x02\xef\x25", "\x02\x9a\x01", "\x02\x6c\x02", "\x02\x0c\x31",
  "\x02\x3e\x01", "\x02\x3c\x01", "\x02\xdb\x24", "\x02\x3d\x1e",
  "\x02\x3c\x01", "\x02\x40\x01", "\x02\x40\x01", "\x02\x37\x1e",
  "\x02\x39\x1e", "\x02\x1a\x03", "\x02\x18\x03", "\x02\x3c\x00",
  "\x02\x64\x22", "\x02\xda\x22", "\x02\x1c\xff", "\x02\x72\x22",
  "\x02\x76\x22", "\x02\x66\x22", "\x02\x64\xfe", "\x02\x6e\x02",
  "\x02\x8c\x25", "\x02\x6d\x02", "\x02\xa4\x20", "\x02\x6c\x05",
  "\x02\xc9\x01", "\x02\x59\x04", "\x02\xc0\xf6", "\x02\x33\x09",
  "\x02\xb3\x0a", "\x02\x3b\x1e", "\x02\x34\x09", "\x02\xe1\x09",
  "\x02\x61\x09", "\x02\xe3\x09", "\x02\x63\x09", "\x02\x6b\x02",
  "\x02\x4c\xff", "\x02\xd0\x33", "\x02\x2c\x0e", "\x02\x27\x22",
  "\x02\xac\x00", "\x02\x10\x23", "\x02\x28\x22", "\x02\x25\x0e",
  "\x02\x7f\x01", "\x02\x4e\xfe", "\x02\x32\x03", "\x02\x4d\xfe",
  "\x02\xca\x25", "\x02\xa7\x24", "\x02\x42\x01", "\x02\x13\x21",
  "\x02\xee\xf6", "\x02\x91\x25", "\x02\x26\x0e", "\x02\x8c\x09",
  "\x02\x0c\x09", "\x02\xe2\x09", "\x02\x62\x09", "\x02\xd3\x33",
  "\x02\x6d\x00", "\x02\xae\x09", "\x02\xaf\x00", "\x02\x31\x03",
  "\x02\x04\x03", "\x02\xcd\x02", "\x02\xe3\xff", "\x02\x3f\x1e",
  "\x02\x2e\x09", "\x02\xae\x0a", "\x02\x2e\x0a", "\x02\xa4\x05",
  "\x02\xa4\x05", "\x02\x7e\x30", "\x02\x95\xf8", "\x02\x94\xf8",
  "\x02\x4b\x0e", "\x02\x93\xf8", "\x02\x8c\xf8", "\x02\x8b\xf8",
  "\x02\x48\x0e", "\x02\x8a\xf8", "\x02\x84\xf8", "\x02\x31\x0e",
  "\x02\x89\xf8", "\x02\x47\x0e", "\x02\x8f\xf8", "\x02\x8e\xf8",
  "\x02\x49\x0e", "\x02\x8d\xf8", "\x02\x92\xf8", "\x02\x91\xf8",
  "\x02\x4a\x0e", "\x02\x90\xf8", "\x02\x46\x0e", "\x02\xde\x30",
  "\x02\x8f\xff", "\x02\x42\x26", "\x02\x47\x33", "\x02\xbe\x05",
  "\x02\x42\x26", "\x02\xaf\x05", "\x02\x83\x33", "\x02\x07\x31",
  "\x02\xd4\x33", "\x02\xdc\x24", "\x02\xa5\x33", "\x02\x41\x1e",
  "\x02\x43\x1e", "\x02\x45\x06", "\x02\xe2\xfe", "\x02\xe3\xfe",
  "\x02\xe4\xfe", "\x02\xd1\xfc", "\x02\x48\xfc", "\x02\x4d\x33",
  "\x02\x81\x30", "\x02\x7e\x33", "\x02\xe1\x30", "\x02\x92\xff",
  "\x02\xde\x05", "\x02\x3e\xfb", "\x02\x3e\xfb", "\x02\xde\x05",
  "\x02\x74\x05", "\x02\xa5\x05", "\x02\xa6\x05", "\x02\xa6\x05",
  "\x02\xa5\x05", "\x02\x71\x02", "\x02\x92\x33", "\x02\x65\xff",
  "\x02\xb7\x00", "\x02\x72\x32", "\x02\x12\x32", "\x02\x64\x32",
  "\x02\x41\x31", "\x02\x70\x31", "\x02\x04\x32", "\x02\x6e\x31",
  "\x02\x6f\x31", "\x02\x7f\x30", "\x02\xdf\x30", "\x02\x90\xff",
  "\x02\x12\x22", "\x02\x20\x03", "\x02\x96\x22", "\x02\xd7\x02",
  "\x02\x13\x22", "\x02\x32\x20", "\x02\x4a\x33", "\x02\x49\x33",
  "\x02\x70\x02", "\x02\x96\x33", "\x02\xa3\x33", "\x02\x4d\xff",
  "\x02\x9f\x33", "\x02\x82\x30", "\x02\xc1\x33", "\x02\xe2\x30",
  "\x02\x93\xff", "\x02\xd6\x33", "\x02\x21\x0e", "\x02\xa7\x33",
  "\x02\xa8\x33", "\x02\xa8\x24", "\x02\xab\x33", "\x02\xb3\x33",
  "\x02\xef\xf6", "\x02\x6f\x02", "\x02\xb5\x00", "\x02\xb5\x00",
  "\x02\x82\x33", "\x02\x6b\x22", "\x02\x6a\x22", "\x02\x8c\x33",
  "\x02\xbc\x03", "\x02\x8d\x33", "\x02\x80\x30", "\x02\xe0\x30",
  "\x02\x91\xff", "\x02\x95\x33", "\x02\xd7\x00", "\x02\x9b\x33",
  "\x02\xa3\x05", "\x02\xa3\x05", "\x02\x6a\x26", "\x02\x6b\x26",
  "\x02\x6d\x26", "\x02\x6f\x26", "\x02\xb2\x33", "\x02\xb6\x33",
  "\x02\xbc\x33", "\x02\xb9\x33", "\x02\xb7\x33", "\x02\xbf\x33",
  "\x02\xbd\x33", "\x02\x6e\x00", "\x02\xa8\x09", "\x02\x07\x22",
  "\x02\x44\x01", "\x02\x28\x09", "\x02\xa8\x0a", "\x02\x28\x0a",
  "\x02\x6a\x30", "\x02\xca\x30", "\x02\x85\xff", "\x02\x49\x01",
  "\x02\x81\x33", "\x02\x0b\x31", "\x02\xa0\x00", "\x02\x48\x01",
  "\x02\x46\x01", "\x02\xdd\x24", "\x02\x4b\x1e", "\x02\x46\x01",
  "\x02\x45\x1e", "\x02\x47\x1e", "\x02\x6d\x30", "\x02\xcd\x30",
  "\x02\x88\xff", "\x02\xaa\x20", "\x02\x8b\x33", "\x02\x99\x09",
  "\x02\x19\x09", "\x02\x99\x0a", "\x02\x19\x0a", "\x02\x07\x0e",
  "\x02\x93\x30", "\x02\x72\x02", "\x02\x73\x02", "\x02\x6f\x32",
  "\x02\x0f\x32", "\x02\x35\x31", "\x02\x61\x32", "\x02\x36\x31",
  "\x02\x34\x31", "\x02\x68\x31", "\x02\x01\x32", "\x02\x67\x31",
  "\x02\x66\x31", "\x02\x6b\x30", "\x02\xcb\x30", "\x02\x86\xff",
  "\x02\x99\xf8", "\x02\x4d\x0e", "\x02\x39\x00", "\x02\x69\x06",
  "\x02\xef\x09", "\x02\x68\x24", "\x02\x92\x27", "\x02\x6f\x09",
  "\x02\xef\x0a", "\x02\x6f\x0a", "\x02\x69\x06", "\x02\x29\x30",
  "\x02\x28\x32", "\x02\x89\x20", "\x02\x19\xff", "\x02\x39\xf7",
  "\x02\x7c\x24", "\x02\x90\x24", "\x02\xf9\x06", "\x02\x78\x21",
  "\x02\x79\x20", "\x02\x72\x24", "\x02\x86\x24", "\x02\x9a\x24",
  "\x02\x59\x0e", "\x02\xcc\x01", "\x02\x5a\x04", "\x02\xf3\x30",
  "\x02\x9d\xff", "\x02\x9e\x01", "\x02\x49\x1e", "\x02\x4e\xff",
  "\x02\x9a\x33", "\x02\xa3\x09", "\x02\x23\x09", "\x02\xa3\x0a",
  "\x02\x23\x0a", "\x02\x29\x09", "\x02\x6e\x30", "\x02\xce\x30",
  "\x02\x89\xff", "\x02\xa0\x00", "\x02\x13\x0e", "\x02\x19\x0e",
  "\x02\x46\x06", "\x02\xe6\xfe", "\x02\xba\x06", "\x02\x9f\xfb",
  "\x04\xe7\xfe\xec\xfe", "\x02\xe7\xfe", "\x02\xd2\xfc", "\x02\x4b\xfc",
  "\x02\xe8\xfe", "\x02\xd5\xfc", "\x02\x4e\xfc", "\x02\x8d\xfc",
  "\x02\x0c\x22", "\x02\x09\x22", "\x02\x09\x22", "\x02\x60\x22",
  "\x02\x6f\x22", "\x02\x71\x22", "\x02\x79\x22", "\x02\x62\x22",
  "\x02\x6e\x22", "\x02\x70\x22", "\x02\x26\x22", "\x02\x80\x22",
  "\x02\x84\x22", "\x02\x81\x22", "\x02\x85\x22", "\x02\x76\x05",
  "\x02\xa9\x24", "\x02\xb1\x33", "\x02\x7f\x20", "\x02\xf1\x00",
  "\x02\xbd\x03", "\x02\x6c\x30", "\x02\xcc\x30", "\x02\x87\xff",
  "\x02\xbc\x09", "\x02\x3c\x09", "\x02\xbc\x0a", "\x02\x3c\x0a",
  "\x02\x23\x00", "\x02\x03\xff", "\x02\x5f\xfe", "\x02\x74\x03",
  "\x02\x75\x03", "\x02\x16\x21", "\x02\xe0\x05", "\x02\x40\xfb",
  "\x02\x40\xfb", "\x02\xe0\x05", "\x02\xb5\x33", "\x02\xbb\x33",
  "\x02\x9e\x09", "\x02\x1e\x09", "\x02\x9e\x0a", "\x02\x1e\x0a",
  "\x02\x6f\x00", "\x02\xf3\x00", "\x02\x2d\x0e", "\x02\x75\x02",
  "\x02\xe9\x04", "\x02\xeb\x04", "\x02\x93\x09", "\x02\x1b\x31",
  "\x02\x4f\x01", "\x02\x11\x09", "\x02\x91\x0a", "\x02\x49\x09",
  "\x02\xc9\x0a", "\x02\xd2\x01", "\x02\xde\x24", "\x02\xf4\x00",
  "\x02\xd1\x1e", "\x02\xd9\x1e", "\x02\xd3\x1e", "\x02\xd5\x1e",
  "\x02\xd7\x1e", "\x02\x3e\x04", "\x02\x51\x01", "\x02\x0d\x02",
  "\x02\x13\x09", "\x02\xf6\x00", "\x02\xe7\x04", "\x02\xcd\x1e",
  "\x02\x53\x01", "\x02\x5a\x31", "\x02\xdb\x02", "\x02\x28\x03",
  "\x02\xf2\x00", "\x02\x93\x0a", "\x02\x85\x05", "\x02\x4a\x30",
  "\x02\xcf\x1e", "\x02\xa1\x01", "\x02\xdb\x1e", "\x02\xe3\x1e",
  "\x02\xdd\x1e", "\x02\xdf\x1e", "\x02\xe1\x1e", "\x02\x51\x01",
  "\x02\xa3\x01", "\x02\x0f\x02", "\x02\xaa\x30", "\x02\x75\xff",
  "\x02\x57\x31", "\x02\xab\x05", "\x02\x4d\x01", "\x02\x53\x1e",
  "\x02\x51\x1e", "\x02\x50\x09", "\x02\xc9\x03", "\x02\xd6\x03",
  "\x02\x61\x04", "\x02\x77\x02", "\x02\x7b\x04", "\x02\x7d\x04",
  "\x02\xce\x03", "\x02\xd0\x0a", "\x02\xbf\x03", "\x02\xcc\x03",
  "\x02\x4f\xff", "\x02\x31\x00", "\x02\x61\x06", "\x02\xe7\x09",
  "\x02\x60\x24", "\x02\x8a\x27", "\x02\x67\x09", "\x02\x24\x20",
  "\x02\x5b\x21", "\x02\xdc\xf6", "\x02\xe7\x0a", "\x02\x67\x0a",
  "\x02\x61\x06", "\x02\xbd\x00", "\x02\x21\x30", "\x02\x20\x32",
  "\x02\x81\x20", "\x02\x11\xff", "\x02\xf4\x09", "\x02\x31\xf7",
  "\x02\x74\x24", "\x02\x88\x24", "\x02\xf1\x06", "\x02\xbc\x00",
  "\x02\x70\x21", "\x02\xb9\x00", "\x02\x51\x0e", "\x02\x53\x21",
  "\x02\xeb\x01", "\x02\xed\x01", "\x02\x13\x0a", "\x02\x4b\x0a",
  "\x02\x54\x02", "\x02\xaa\x24", "\x02\xe6\x25", "\x02\x25\x23",
  "\x02\xaa\x00", "\x02\xba\x00", "\x02\x1f\x22", "\x02\x12\x09",
  "\x02\x4a\x09", "\x02\xf8\x00", "\x02\xff\x01", "\x02\x49\x30",
  "\x02\xa9\x30", "\x02\x6b\xff", "\x02\xff\x01", "\x02\xf0\xf6",
  "\x02\x7f\x04", "\x02\xf5\x00", "\x02\x4d\x1e", "\x02\x4f\x1e",
  "\x02\x21\x31", "\x02\x3e\x20", "\x02\x4a\xfe", "\x02\x05\x03",
  "\x02\x49\xfe", "\x02\x4c\xfe", "\x02\x4b\xfe", "\x02\xaf\x00",
  "\x02\xcb\x09", "\x02\x4b\x09", "\x02\xcb\x0a", "\x02\x70\x00",
  "\x02\x80\x33", "\x02\x2b\x33", "\x02\xaa\x09", "\x02\x55\x1e",
  "\x02\x2a\x09", "\x02\xdf\x21", "\x02\xde\x21", "\x02\xaa\x0a",
  "\x02\x2a\x0a", "\x02\x71\x30", "\x02\x2f\x0e", "\x02\xd1\x30",
  "\x02\x84\x04", "\x02\xc0\x04", "\x02\x7f\x31", "\x02\xb6\x00",
  "\x02\x25\x22", "\x02\x28\x00", "\x02\x3e\xfd", "\x02\xed\xf8",
  "\x02\xec\xf8", "\x02\x8d\x20", "\x02\x08\xff", "\x02\x59\xfe",
  "\x02\x7d\x20", "\x02\xeb\xf8", "\x02\x35\xfe", "\x02\x29\x00",
  "\x02\x3f\xfd", "\x02\xf8\xf8", "\x02\xf7\xf8", "\x02\x8e\x20",
  "\x02\x09\xff", "\x02\x5a\xfe", "\x02\x7e\x20", "\x02\xf6\xf8",
  "\x02\x36\xfe", "\x02\x02\x22", "\x02\xc0\x05", "\x02\x99\x05",
  "\x02\xa9\x33", "\x02\xb7\x05", "\x02\xb7\x05", "\x02\xb7\x05",
  "\x02\xb7\x05", "\x02\xb7\x05", "\x02\xb7\x05", "\x02\xb7\x05",
  "\x02\xb7\x05", "\x02\xa1\x05", "\x02\x06\x31", "\x02\xdf\x24",
  "\x02\x57\x1e", "\x02\xe4\x05", "\x02\x3f\x04", "\x02\x44\xfb",
  "\x02\x44\xfb", "\x02\x3b\x33", "\x02\x43\xfb", "\x02\x7e\x06",
  "\x02\x7a\x05", "\x02\xe4\x05", "\x02\x57\xfb", "\x02\x58\xfb",
  "\x02\x7a\x30", "\x02\x59\xfb", "\x02\xda\x30", "\x02\xa7\x04",
  "\x02\x4e\xfb", "\x02\x25\x00", "\x02\x6a\x06", "\x02\x05\xff",
  "\x02\x6a\xfe", "\x02\x2e\x00", "\x02\x89\x05", "\x02\xb7\x00",
  "\x02\x61\xff", "\x02\xe7\xf6", "\x02\x0e\xff", "\x02\x52\xfe",
  "\x02\xe8\xf6", "\x02\x42\x03", "\x02\xa5\x22", "\x02\x30\x20",
  "\x02\xa7\x20", "\x02\x8a\x33", "\x02\xab\x09", "\x02\x2b\x09",
  "\x02\xab\x0a", "\x02\x2b\x0a", "\x02\xc6\x03", "\x02\xd5\x03",
  "\x02\x7a\x32", "\x02\x1a\x32", "\x02\x6c\x32", "\x02\x4d\x31",
  "\x02\x0c\x32", "\x02\x78\x02", "\x02\x3a\x0e", "\x02\xd5\x03",
  "\x02\xa5\x01", "\x02\x1e\x0e", "\x02\x1c\x0e", "\x02\x20\x0e",
  "\x02\xc0\x03", "\x02\x73\x32", "\x02\x13\x32", "\x02\x76\x31",
  "\x02\x65\x32", "\x02\x72\x31", "\x02\x42\x31", "\x02\x05\x32",
  "\x02\x74\x31", "\x02\x44\x31", "\x02\x75\x31", "\x02\x77\x31",
  "\x02\x73\x31", "\x02\x74\x30", "\x02\xd4\x30", "\x02\xd6\x03",
  "\x02\x83\x05", "\x02\x2b\x00", "\x02\x1f\x03", "\x02\x95\x22",
  "\x02\xb1\x00", "\x02\xd6\x02", "\x02\x0b\xff", "\x02\x62\xfe",
  "\x02\x7a\x20", "\x02\x50\xff", "\x02\xd8\x33", "\x02\x7d\x30",
  "\x02\x1f\x26", "\x02\x1c\x26", "\x02\x1e\x26", "\x02\x1d\x26",
  "\x02\xdd\x30", "\x02\x1b\x0e", "\x02\x12\x30", "\x02\x20\x30",
  "\x02\xab\x24", "\x02\x7a\x22", "\x02\x1e\x21", "\x02\xb9\x02",
  "\x02\x35\x20", "\x02\x0f\x22", "\x02\x05\x23", "\x02\xfc\x30",
  "\x02\x18\x23", "\x02\x82\x22", "\x02\x83\x22", "\x02\x37\x22",
  "\x02\x1d\x22", "\x02\xc8\x03", "\x02\x71\x04", "\x02\x86\x04",
  "\x02\xb0\x33", "\x02\x77\x30", "\x02\xd7\x30", "\x02\xb4\x33",
  "\x02\xba\x33", "\x02\x71\x00", "\x02\x58\x09", "\x02\xa8\x05",
  "\x02\x42\x06", "\x02\xd6\xfe", "\x02\xd7\xfe", "\x02\xd8\xfe",
  "\x02\xb8\x05", "\x02\xb8\x05", "\x02\xb8\x05", "\x02\xb8\x05",
  "\x02\xb8\x05", "\x02\xb8\x05", "\x02\xb8\x05", "\x02\xb8\x05",
  "\x02\xb8\x05", "\x02\xb8\x05", "\x02\xb8\x05", "\x02\xb8\x05",
  "\x02\xb8\x05", "\x02\xb8\x05", "\x02\xb8\x05", "\x02\xb8\x05",
  "\x02\x9f\x05", "\x02\x11\x31", "\x02\xe0\x24", "\x02\xa0\x02",
  "\x02\x51\xff", "\x02\xe7\x05", "\x02\x47\xfb", "\x02\x47\xfb",
  "\x04\xe7\x05\xb2\x05", "\x04\xe7\x05\xb2\x05", "\x04\xe7\x05\xb1\x05",
  "\x04\xe7\x05\xb1\x05", "\x02\xe7\x05", "\x04\xe7\x05\xb4\x05",
  "\x04\xe7\x05\xb4\x05", "\x04\xe7\x05\xb9\x05", "\x04\xe7\x05\xb9\x05",
  "\x04\xe7\x05\xb7\x05", "\x04\xe7\x05\xb7\x05", "\x04\xe7\x05\xb8\x05",
  "\x04\xe7\x05\xb8\x05", "\x04\xe7\x05\xbb\x05", "\x04\xe7\x05\xbb\x05",
  "\x04\xe7\x05\xb6\x05", "\x04\xe7\x05\xb6\x05", "\x04\xe7\x05\xb0\x05",
  "\x04\xe7\x05\xb0\x05", "\x04\xe7\x05\xb5\x05", "\x04\xe7\x05\xb5\x05",
  "\x02\xac\x24", "\x02\x69\x26", "\x02\xbb\x05", "\x02\xbb\x05",
  "\x02\xbb\x05", "\x02\xbb\x05", "\x02\xbb\x05", "\x02\xbb\x05",
  "\x02\xbb\x05", "\x02\xbb\x05", "\x02\x3f\x00", "\x02\x1f\x06",
  "\x02\x5e\x05", "\x02\xbf\x00", "\x02\xbf\xf7", "\x02\x7e\x03",
  "\x02\x1f\xff", "\x02\x3f\xf7", "\x02\x22\x00", "\x02\x1e\x20",
  "\x02\x1c\x20", "\x02\x02\xff", "\x02\x1e\x30", "\x02\x1d\x30",
  "\x02\x1d\x20", "\x02\x18\x20", "\x02\x1b\x20", "\x02\x1b\x20",
  "\x02\x19\x20", "\x02\x49\x01", "\x02\x1a\x20", "\x02\x27\x00",
  "\x02\x07\xff", "\x02\x72\x00", "\x02\x7c\x05", "\x02\xb0\x09",
  "\x02\x55\x01", "\x02\x30\x09", "\x02\x1a\x22", "\x02\xe5\xf8",
  "\x02\xae\x33", "\x02\xaf\x33", "\x02\xad\x33", "\x02\xbf\x05",
  "\x02\xbf\x05", "\x02\xb0\x0a", "\x02\x30\x0a", "\x02\x89\x30",
  "\x02\xe9\x30", "\x02\x97\xff", "\x02\xf1\x09", "\x02\xf0\x09",
  "\x02\x64\x02", "\x02\x36\x22", "\x02\x16\x31", "\x02\x59\x01",
  "\x02\x57\x01", "\x02\xe1\x24", "\x02\x57\x01", "\x02\x11\x02",
  "\x02\x59\x1e", "\x02\x5b\x1e", "\x02\x5d\x1e", "\x02\x3b\x20",
  "\x02\x86\x22", "\x02\x87\x22", "\x02\xae\x00", "\x02\xe8\xf8",
  "\x02\xda\xf6", "\x02\x31\x06", "\x02\x80\x05", "\x02\xae\xfe",
  "\x02\x8c\x30", "\x08\x31\x06\xf3\xfe\x8e\xfe\x44\x06", "\x02\xec\x30",
  "\x02\x9a\xff", "\x02\xe8\x05", "\x02\x48\xfb", "\x04\xe8\x05\xb2\x05",
  "\x04\xe8\x05\xb2\x05", "\x04\xe8\x05\xb1\x05", "\x04\xe8\x05\xb1\x05",
  "\x02\xe8\x05", "\x04\xe8\x05\xb4\x05", "\x04\xe8\x05\xb4\x05",
  "\x04\xe8\x05\xb9\x05", "\x04\xe8\x05\xb9\x05", "\x04\xe8\x05\xb7\x05",
  "\x04\xe8\x05\xb7\x05", "\x04\xe8\x05\xb8\x05", "\x04\xe8\x05\xb8\x05",
  "\x04\xe8\x05\xbb\x05", "\x04\xe8\x05\xbb\x05", "\x04\xe8\x05\xb6\x05",
  "\x04\xe8\x05\xb6\x05", "\x04\xe8\x05\xb0\x05", "\x04\xe8\x05\xb0\x05",
  "\x04\xe8\x05\xb5\x05", "\x04\xe8\x05\xb5\x05", "\x02\x3d\x22",
  "\x02\x97\x05", "\x02\x97\x05", "\x02\x10\x23", "\x02\x7e\x02",
  "\x02\x7f\x02", "\x02\xdd\x09", "\x02\x5d\x09", "\x02\xc1\x03",
  "\x02\x7d\x02", "\x02\x7b\x02", "\x02\xb5\x02", "\x02\xf1\x03",
  "\x02\xde\x02", "\x02\x71\x32", "\x02\x11\x32", "\x02\x63\x32",
  "\x02\x40\x31", "\x02\x3a\x31", "\x02\x69\x31", "\x02\x39\x31",
  "\x02\x3b\x31", "\x02\x6c\x31", "\x02\x03\x32", "\x02\x3f\x31",
  "\x02\x3c\x31", "\x02\x6b\x31", "\x02\x3d\x31", "\x02\x3e\x31",
  "\x02\x6a\x31", "\x02\x6d\x31", "\x02\x1f\x22", "\x02\x19\x03",
  "\x02\xbf\x22", "\x02\x8a\x30", "\x02\xea\x30", "\x02\x98\xff",
  "\x02\xda\x02", "\x02\x25\x03", "\x02\x0a\x03", "\x02\xbf\x02",
  "\x02\x59\x05", "\x02\x1c\x03", "\x02\xd3\x02", "\x02\xbe\x02",
  "\x02\x39\x03", "\x02\xd2\x02", "\x02\x13\x02", "\x02\x51\x33",
  "\x02\x5f\x1e", "\x02\x7c\x02", "\x02\x7a\x02", "\x02\x52\xff",
  "\x02\x8d\x30", "\x02\xed\x30", "\x02\x9b\xff", "\x02\x23\x0e",
  "\x02\xad\x24", "\x02\xdc\x09", "\x02\x31\x09", "\x02\x5c\x0a",
  "\x02\x91\x06", "\x02\x8d\xfb", "\x02\xe0\x09", "\x02\x60\x09",
  "\x02\xe0\x0a", "\x02\xc4\x09", "\x02\x44\x09", "\x02\xc4\x0a",
  "\x02\xf1\xf6", "\x02\x90\x25", "\x02\x79\x02", "\x02\xb4\x02",
  "\x02\x8b\x30", "\x02\xeb\x30", "\x02\x99\xff", "\x02\xf2\x09",
  "\x02\xf3\x09", "\x02\xdd\xf6", "\x02\x24\x0e", "\x02\x8b\x09",
  "\x02\x0b\x09", "\x02\x8b\x0a", "\x02\xc3\x09", "\x02\x43\x09",
  "\x02\xc3\x0a", "\x02\x73\x00", "\x02\xb8\x09", "\x02\x5b\x01",
  "\x02\x65\x1e", "\x02\x35\x06", "\x02\x38\x09", "\x02\xba\xfe",
  "\x02\xbb\xfe", "\x02\xbc\xfe", "\x02\xb8\x0a", "\x02\x38\x0a",
  "\x02\x55\x30", "\x02\xb5\x30", "\x02\x7b\xff", "\x02\xfa\xfd",
  "\x02\xe1\x05", "\x02\x41\xfb", "\x02\x41\xfb", "\x02\xe1\x05",
  "\x02\x32\x0e", "\x02\x41\x0e", "\x02\x44\x0e", "\x02\x43\x0e",
  "\x02\x33\x0e", "\x02\x30\x0e", "\x02\x40\x0e", "\x02\x86\xf8",
  "\x02\x35\x0e", "\x02\x85\xf8", "\x02\x34\x0e", "\x02\x42\x0e",
  "\x02\x88\xf8", "\x02\x37\x0e", "\x02\x87\xf8", "\x02\x36\x0e",
  "\x02\x38\x0e", "\x02\x39\x0e", "\x02\x19\x31", "\x02\x61\x01",
  "\x02\x67\x1e", "\x02\x5f\x01", "\x02\x59\x02", "\x02\xd9\x04",
  "\x02\xdb\x04", "\x02\x5a\x02", "\x02\xe2\x24", "\x02\x5d\x01",
  "\x02\x19\x02", "\x02\x61\x1e", "\x02\x63\x1e", "\x02\x69\x1e",
  "\x02\x3c\x03", "\x02\x33\x20", "\x02\xca\x02", "\x02\xa7\x00",
  "\x02\x33\x06", "\x02\xb2\xfe", "\x02\xb3\xfe", "\x02\xb4\xfe",
  "\x02\xb6\x05", "\x02\xb6\x05", "\x02\xb6\x05", "\x02\xb6\x05",
  "\x02\xb6\x05", "\x02\xb6\x05", "\x02\xb6\x05", "\x02\x92\x05",
  "\x02\xb6\x05", "\x02\x7d\x05", "\x02\x5b\x30", "\x02\xbb\x30",
  "\x02\x7e\xff", "\x02\x3b\x00", "\x02\x1b\x06", "\x02\x1b\xff",
  "\x02\x54\xfe", "\x02\x9c\x30", "\x02\x9f\xff", "\x02\x22\x33",
  "\x02\x23\x33", "\x02\x37\x00", "\x02\x67\x06", "\x02\xed\x09",
  "\x02\x66\x24", "\x02\x90\x27", "\x02\x6d\x09", "\x02\x5e\x21",
  "\x02\xed\x0a", "\x02\x6d\x0a", "\x02\x67\x06", "\x02\x27\x30",
  "\x02\x26\x32", "\x02\x87\x20", "\x02\x17\xff", "\x02\x37\xf7",
  "\x02\x7a\x24", "\x02\x8e\x24", "\x02\xf7\x06", "\x02\x76\x21",
  "\x02\x77\x20", "\x02\x70\x24", "\x02\x84\x24", "\x02\x98\x24",
  "\x02\x57\x0e", "\x02\xad\x00", "\x02\x77\x05", "\x02\xb6\x09",
  "\x02\x48\x04", "\x02\x51\x06", "\x02\x61\xfc", "\x02\x5e\xfc",
  "\x02\x60\xfc", "\x04\x51\x06\x4b\x06", "\x02\x62\xfc", "\x02\x5f\xfc",
  "\x02\x92\x25", "\x02\x93\x25", "\x02\x91\x25", "\x02\x92\x25",
  "\x02\x36\x09", "\x02\xb6\x0a", "\x02\x36\x0a", "\x02\x93\x05",
  "\x02\x15\x31", "\x02\x49\x04", "\x02\x34\x06", "\x02\xb6\xfe",
  "\x02\xb7\xfe", "\x02\xb8\xfe", "\x02\xe3\x03", "\x02\xaa\x20",
  "\x02\xaa\x20", "\x02\xb0\x05", "\x02\xb0\x05", "\x02\xb0\x05",
  "\x02\xb0\x05", "\x02\xb0\x05", "\x02\xb0\x05", "\x02\xb0\x05",
  "\x02\xb0\x05", "\x02\xb0\x05", "\x02\xbb\x04", "\x02\xed\x03",
  "\x02\xe9\x05", "\x02\x49\xfb", "\x02\x49\xfb", "\x02\x2c\xfb",
  "\x02\x2c\xfb", "\x02\x2d\xfb", "\x02\x2d\xfb", "\x02\xc1\x05",
  "\x02\xe9\x05", "\x02\x2a\xfb", "\x02\x2a\xfb", "\x02\x2b\xfb",
  "\x02\x2b\xfb", "\x02\x82\x02", "\x02\xc3\x03", "\x02\xc2\x03",
  "\x02\xc2\x03", "\x02\xf2\x03", "\x02\x57\x30", "\x02\xb7\x30",
  "\x02\x7c\xff", "\x02\xbd\x05", "\x02\xbd\x05", "\x02\x3c\x22",
  "\x02\xc2\x05", "\x02\x74\x32", "\x02\x14\x32", "\x02\x7e\x31",
  "\x02\x66\x32", "\x02\x7a\x31", "\x02\x45\x31", "\x02\x7b\x31",
  "\x02\x06\x32", "\x02\x7d\x31", "\x02\x7c\x31", "\x02\x36\x00",
  "\x02\x66\x06", "\x02\xec\x09", "\x02\x65\x24", "\x02\x8f\x27",
  "\x02\x6c\x09", "\x02\xec\x0a", "\x02\x6c\x0a", "\x02\x66\x06",
  "\x02\x26\x30", "\x02\x25\x32", "\x02\x86\x20", "\x02\x16\xff",
  "\x02\x36\xf7", "\x02\x79\x24", "\x02\x8d\x24", "\x02\xf6\x06",
  "\x02\x75\x21", "\x02\x76\x20", "\x02\x6f\x24", "\x02\xf9\x09",
  "\x02\x83\x24", "\x02\x97\x24", "\x02\x56\x0e", "\x02\x2f\x00",
  "\x02\x0f\xff", "\x02\x7f\x01", "\x02\x9b\x1e", "\x02\x3a\x26",
  "\x02\x53\xff", "\x02\xc3\x05", "\x02\xad\x00", "\x02\x4c\x04",
  "\x02\x5d\x30", "\x02\xbd\x30", "\x02\x7f\xff", "\x02\x38\x03",
  "\x02\x37\x03", "\x02\x29\x0e", "\x02\x28\x0e", "\x02\x0b\x0e",
  "\x02\x2a\x0e", "\x02\x20\x00", "\x02\x20\x00", "\x02\x60\x26",
  "\x02\x60\x26", "\x02\x64\x26", "\x02\xae\x24", "\x02\x3b\x03",
  "\x02\xc4\x33", "\x02\x9d\x33", "\x02\xa9\x25", "\x02\xa4\x25",
  "\x02\x8f\x33", "\x02\x9e\x33", "\x02\xce\x33", "\x02\xd1\x33",
  "\x02\xd2\x33", "\x02\x8e\x33", "\x02\xd5\x33", "\x02\x9c\x33",
  "\x02\xa1\x33", "\x02\xa6\x25", "\x02\xa7\x25", "\x02\xa8\x25",
  "\x02\xa5\x25", "\x02\xa3\x25", "\x02\xdb\x33", "\x02\xb7\x09",
  "\x02\x37\x09", "\x02\xb7\x0a", "\x02\x49\x31", "\x02\x85\x31",
  "\x02\x80\x31", "\x02\x32\x31", "\x02\x65\x31", "\x02\x43\x31",
  "\x02\x46\x31", "\x02\x38\x31", "\x02\xf2\xf6", "\x02\xa3\x00",
  "\x02\xe1\xff", "\x02\x36\x03", "\x02\x35\x03", "\x02\x82\x22",
  "\x02\x8a\x22", "\x02\x86\x22", "\x02\x7b\x22", "\x02\x0b\x22",
  "\x02\x59\x30", "\x02\xb9\x30", "\x02\x7d\xff", "\x02\x52\x06",
  "\x02\x11\x22", "\x02\x3c\x26", "\x02\x83\x22", "\x02\x8b\x22",
  "\x02\x87\x22", "\x02\xdc\x33", "\x02\x7c\x33", "\x02\x74\x00",
  "\x02\xa4\x09", "\x02\xa4\x22", "\x02\xa3\x22", "\x02\x24\x09",
  "\x02\xa4\x0a", "\x02\x24\x0a", "\x02\x37\x06", "\x02\xc2\xfe",
  "\x02\xc3\xfe", "\x02\x5f\x30", "\x02\xc4\xfe", "\x02\x7d\x33",
  "\x02\xbf\x30", "\x02\x80\xff", "\x02\x40\x06", "\x02\xc4\x03",
  "\x02\xea\x05", "\x02\x4a\xfb", "\x02\x4a\xfb", "\x02\x4a\xfb",
  "\x02\xea\x05", "\x02\x67\x01", "\x02\x0a\x31", "\x02\x65\x01",
  "\x02\xa8\x02", "\x02\x63\x01", "\x02\x86\x06", "\x02\x7b\xfb",
  "\x02\x7c\xfb", "\x02\x7d\xfb", "\x04\x7c\xfb\xe4\xfe", "\x02\xe3\x24",
  "\x02\x71\x1e", "\x02\x63\x01", "\x02\x97\x1e", "\x02\x6b\x1e",
  "\x02\x6d\x1e", "\x02\x42\x04", "\x02\xad\x04", "\x02\x2a\x06",
  "\x02\x96\xfe", "\x02\xa2\xfc", "\x02\x0c\xfc", "\x02\x97\xfe",
  "\x02\x66\x30", "\x02\xa1\xfc", "\x02\x0b\xfc", "\x02\x29\x06",
  "\x02\x94\xfe", "\x02\x98\xfe", "\x02\xa4\xfc", "\x02\x0e\xfc",
  "\x02\x73\xfc", "\x02\xc6\x30", "\x02\x83\xff", "\x02\x21\x21",
  "\x02\x0e\x26", "\x02\xa0\x05", "\x02\xa9\x05", "\x02\x69\x24",
  "\x02\x29\x32", "\x02\x7d\x24", "\x02\x91\x24", "\x02\x79\x21",
  "\x02\xa7\x02", "\x02\xd8\x05", "\x02\x38\xfb", "\x02\x38\xfb",
  "\x02\xd8\x05", "\x02\xb5\x04", "\x02\x9b\x05", "\x02\x9b\x05",
  "\x02\xa5\x09", "\x02\x25\x09", "\x02\xa5\x0a", "\x02\x25\x0a",
  "\x02\x30\x06", "\x02\xac\xfe", "\x02\x98\xf8", "\x02\x97\xf8",
  "\x02\x4c\x0e", "\x02\x96\xf8", "\x02\x2b\x06", "\x02\x9a\xfe",
  "\x02\x9b\xfe", "\x02\x9c\xfe", "\x02\x03\x22", "\x02\x34\x22",
  "\x02\xb8\x03", "\x02\xd1\x03", "\x02\xd1\x03", "\x02\x79\x32",
  "\x02\x19\x32", "\x02\x6b\x32", "\x02\x4c\x31", "\x02\x0b\x32",
  "\x02\x6c\x24", "\x02\x80\x24", "\x02\x94\x24", "\x02\x11\x0e",
  "\x02\xad\x01", "\x02\x12\x0e", "\x02\xfe\x00", "\x02\x17\x0e",
  "\x02\x10\x0e", "\x02\x18\x0e", "\x02\x16\x0e", "\x02\x82\x04",
  "\x02\x6c\x06", "\x02\x6c\x06", "\x02\x33\x00", "\x02\x63\x06",
  "\x02\xe9\x09", "\x02\x62\x24", "\x02\x8c\x27", "\x02\x69\x09",
  "\x02\x5c\x21", "\x02\xe9\x0a", "\x02\x69\x0a", "\x02\x63\x06",
  "\x02\x23\x30", "\x02\x22\x32", "\x02\x83\x20", "\x02\x13\xff",
  "\x02\xf6\x09", "\x02\x33\xf7", "\x02\x76\x24", "\x02\x8a\x24",
  "\x02\xf3\x06", "\x02\xbe\x00", "\x02\xde\xf6", "\x02\x72\x21",
  "\x02\xb3\x00", "\x02\x53\x0e", "\x02\x94\x33", "\x02\x61\x30",
  "\x02\xc1\x30", "\x02\x81\xff", "\x02\x70\x32", "\x02\x10\x32",
  "\x02\x62\x32", "\x02\x37\x31", "\x02\x02\x32", "\x02\xdc\x02",
  "\x02\x30\x03", "\x02\x03\x03", "\x02\x03\x03", "\x02\x60\x03",
  "\x02\x3c\x22", "\x02\x34\x03", "\x02\x3e\x03", "\x02\x97\x22",
  "\x02\x96\x05", "\x02\x96\x05", "\x02\x70\x0a", "\x02\x83\x04",
  "\x02\x7f\x05", "\x02\x6f\x1e", "\x02\x54\xff", "\x02\x69\x05",
  "\x02\x68\x30", "\x02\xc8\x30", "\x02\x84\xff", "\x02\xe5\x02",
  "\x02\xe9\x02", "\x02\xe6\x02", "\x02\xe8\x02", "\x02\xe7\x02",
  "\x02\xbd\x01", "\x02\x85\x01", "\x02\xa8\x01", "\x02\x84\x03",
  "\x02\x27\x33", "\x02\x0f\x0e", "\x02\x14\x30", "\x02\x5d\xfe",
  "\x02\x39\xfe", "\x02\x15\x30", "\x02\x5e\xfe", "\x02\x3a\xfe",
  "\x02\x15\x0e", "\x02\xab\x01", "\x02\xaf\x24", "\x02\x22\x21",
  "\x02\xea\xf8", "\x02\xdb\xf6", "\x02\x88\x02", "\x02\xbc\x25",
  "\x02\xc4\x25", "\x02\xba\x25", "\x02\xb2\x25", "\x02\xa6\x02",
  "\x02\xe6\x05", "\x02\x46\xfb", "\x02\x46\xfb", "\x02\xe6\x05",
  "\x02\x46\x04", "\x02\xb5\x05", "\x02\xb5\x05", "\x02\xb5\x05",
  "\x02\xb5\x05", "\x02\xb5\x05", "\x02\xb5\x05", "\x02\xb5\x05",
  "\x02\xb5\x05", "\x02\x5b\x04", "\x02\xf3\xf6", "\x02\x9f\x09",
  "\x02\x1f\x09", "\x02\x9f\x0a", "\x02\x1f\x0a", "\x02\x79\x06",
  "\x02\x67\xfb", "\x02\x68\xfb", "\x02\x69\xfb", "\x02\xa0\x09",
  "\x02\x20\x09", "\x02\xa0\x0a", "\x02\x20\x0a", "\x02\x87\x02",
  "\x02\x64\x30", "\x02\xc4\x30", "\x02\x82\xff", "\x02\x63\x30",
  "\x02\xc3\x30", "\x02\x6f\xff", "\x02\x6b\x24", "\x02\x7f\x24",
  "\x02\x93\x24", "\x02\x7b\x21", "\x02\x73\x24", "\x02\x44\x53",
  "\x02\x87\x24", "\x02\x9b\x24", "\x02\x32\x00", "\x02\x62\x06",
  "\x02\xe8\x09", "\x02\x61\x24", "\x02\x8b\x27", "\x02\x68\x09",
  "\x02\x25\x20", "\x02\x25\x20", "\x02\x30\xfe", "\x02\xe8\x0a",
  "\x02\x68\x0a", "\x02\x62\x06", "\x02\x22\x30", "\x02\x21\x32",
  "\x02\x82\x20", "\x02\x12\xff", "\x02\xf5\x09", "\x02\x32\xf7",
  "\x02\x75\x24", "\x02\x89\x24", "\x02\xf2\x06", "\x02\x71\x21",
  "\x02\xbb\x01", "\x02\xb2\x00", "\x02\x52\x0e", "\x02\x54\x21",
  "\x02\x75\x00", "\x02\xfa\x00", "\x02\x89\x02", "\x02\x89\x09",
  "\x02\x28\x31", "\x02\x6d\x01", "\x02\xd4\x01", "\x02\xe4\x24",
  "\x02\xfb\x00", "\x02\x77\x1e", "\x02\x43\x04", "\x02\x51\x09",
  "\x02\x71\x01", "\x02\x15\x02", "\x02\x09\x09", "\x02\xfc\x00",
  "\x02\xd8\x01", "\x02\x73\x1e", "\x02\xda\x01", "\x02\xf1\x04",
  "\x02\xdc\x01", "\x02\xd6\x01", "\x02\xe5\x1e", "\x02\xf9\x00",
  "\x02\x89\x0a", "\x02\x09\x0a", "\x02\x46\x30", "\x02\xe7\x1e",
  "\x02\xb0\x01", "\x02\xe9\x1e", "\x02\xf1\x1e", "\x02\xeb\x1e",
  "\x02\xed\x1e", "\x02\xef\x1e", "\x02\x71\x01", "\x02\xf3\x04",
  "\x02\x17\x02", "\x02\xa6\x30", "\x02\x73\xff", "\x02\x79\x04",
  "\x02\x5c\x31", "\x02\x6b\x01", "\x02\xef\x04", "\x02\x7b\x1e",
  "\x02\x41\x0a", "\x02\x55\xff", "\x02\x5f\x00", "\x02\x17\x20",
  "\x02\x3f\xff", "\x02\x33\xfe", "\x02\x4f\xfe", "\x02\x2a\x22",
  "\x02\x00\x22", "\x02\x73\x01", "\x02\xb0\x24", "\x02\x80\x25",
  "\x02\xc4\x05", "\x02\xc5\x03", "\x02\xcb\x03", "\x02\xb0\x03",
  "\x02\x8a\x02", "\x02\xcd\x03", "\x02\x1d\x03", "\x02\xd4\x02",
  "\x02\x73\x0a", "\x02\x6f\x01", "\x02\x5e\x04", "\x02\x45\x30",
  "\x02\xa5\x30", "\x02\x69\xff", "\x02\xaf\x04", "\x02\xb1\x04",
  "\x02\x69\x01", "\x02\x79\x1e", "\x02\x75\x1e", "\x02\x8a\x09",
  "\x02\x0a\x09", "\x02\x8a\x0a", "\x02\x0a\x0a", "\x02\x42\x0a",
  "\x02\xc2\x09", "\x02\x42\x09", "\x02\xc2\x0a", "\x02\xc1\x09",
  "\x02\x41\x09", "\x02\xc1\x0a", "\x02\x76\x00", "\x02\x35\x09",
  "\x02\xb5\x0a", "\x02\x35\x0a", "\x02\xf7\x30", "\x02\xd5\x05",
  "\x02\x35\xfb", "\x02\x35\xfb", "\x02\x35\xfb", "\x02\xd5\x05",
  "\x02\x4b\xfb", "\x02\x4b\xfb", "\x02\xf0\x05", "\x02\xf1\x05",
  "\x02\xe5\x24", "\x02\x7f\x1e", "\x02\x32\x04", "\x02\xa4\x06",
  "\x02\x6b\xfb", "\x02\x6c\xfb", "\x02\x6d\xfb", "\x02\xf9\x30",
  "\x02\x40\x26", "\x02\x7c\x00", "\x02\x0d\x03", "\x02\x29\x03",
  "\x02\xcc\x02", "\x02\xc8\x02", "\x02\x7e\x05", "\x02\x8b\x02",
  "\x02\xf8\x30", "\x02\xcd\x09", "\x02\x4d\x09", "\x02\xcd\x0a",
  "\x02\x83\x09", "\x02\x03\x09", "\x02\x83\x0a", "\x02\x56\xff",
  "\x02\x78\x05", "\x02\x9e\x30", "\x02\xfe\x30", "\x02\x9b\x30",
  "\x02\x9e\xff", "\x02\xfa\x30", "\x02\xb1\x24", "\x02\x7d\x1e",
  "\x02\x8c\x02", "\x02\x94\x30", "\x02\xf4\x30", "\x02\x77\x00",
  "\x02\x83\x1e", "\x02\x59\x31", "\x02\x8f\x30", "\x02\xef\x30",
  "\x02\x9c\xff", "\x02\x58\x31", "\x02\x8e\x30", "\x02\xee\x30",
  "\x02\x57\x33", "\x02\x1c\x30", "\x02\x34\xfe", "\x02\x48\x06",
  "\x02\xee\xfe", "\x02\x24\x06", "\x02\x86\xfe", "\x02\xdd\x33",
  "\x02\xe6\x24", "\x02\x75\x01", "\x02\x85\x1e", "\x02\x87\x1e",
  "\x02\x89\x1e", "\x02\x91\x30", "\x02\x18\x21", "\x02\xf1\x30",
  "\x02\x5e\x31", "\x02\x5d\x31", "\x02\x81\x1e", "\x02\xe6\x25",
  "\x02\xcb\x25", "\x02\xd9\x25", "\x02\x0e\x30", "\x02\x43\xfe",
  "\x02\x0f\x30", "\x02\x44\xfe", "\x02\xc7\x25", "\x02\xc8\x25",
  "\x02\xbf\x25", "\x02\xbd\x25", "\x02\xc3\x25", "\x02\xc1\x25",
  "\x02\x16\x30", "\x02\x17\x30", "\x02\xb9\x25", "\x02\xb7\x25",
  "\x02\xab\x25", "\x02\x3a\x26", "\x02\xa1\x25", "\x02\x06\x26",
  "\x02\x0f\x26", "\x02\x18\x30", "\x02\x19\x30", "\x02\xb5\x25",
  "\x02\xb3\x25", "\x02\x90\x30", "\x02\xf0\x30", "\x02\x5f\x31",
  "\x02\x57\xff", "\x02\x92\x30", "\x02\xf2\x30", "\x02\x66\xff",
  "\x02\xa9\x20", "\x02\xe6\xff", "\x02\x27\x0e", "\x02\xb2\x24",
  "\x02\x98\x1e", "\x02\xb7\x02", "\x02\x8d\x02", "\x02\xbf\x01",
  "\x02\x78\x00", "\x02\x3d\x03", "\x02\x12\x31", "\x02\xe7\x24",
  "\x02\x8d\x1e", "\x02\x8b\x1e", "\x02\x6d\x05", "\x02\xbe\x03",
  "\x02\x58\xff", "\x02\xb3\x24", "\x02\xe3\x02", "\x02\x79\x00",
  "\x02\x4e\x33", "\x02\xaf\x09", "\x02\xfd\x00", "\x02\x2f\x09",
  "\x02\x52\x31", "\x02\xaf\x0a", "\x02\x2f\x0a", "\x02\x84\x30",
  "\x02\xe4\x30", "\x02\x94\xff", "\x02\x51\x31", "\x02\x4e\x0e",
  "\x02\x83\x30", "\x02\xe3\x30", "\x02\x6c\xff", "\x02\x63\x04",
  "\x02\xe8\x24", "\x02\x77\x01", "\x02\xff\x00", "\x02\x8f\x1e",
  "\x02\xf5\x1e", "\x02\x4a\x06", "\x02\xd2\x06", "\x02\xaf\xfb",
  "\x02\xf2\xfe", "\x02\x26\x06", "\x02\x8a\xfe", "\x02\x8b\xfe",
  "\x02\x8c\xfe", "\x02\xf3\xfe", "\x02\xf4\xfe", "\x02\xdd\xfc",
  "\x02\x58\xfc", "\x02\x94\xfc", "\x02\xd1\x06", "\x02\x56\x31",
  "\x02\xa5\x00", "\x02\xe5\xff", "\x02\x55\x31", "\x02\x86\x31",
  "\x02\xaa\x05", "\x02\xaa\x05", "\x02\x4b\x04", "\x02\xf9\x04",
  "\x02\x81\x31", "\x02\x83\x31", "\x02\x82\x31", "\x02\x9a\x05",
  "\x02\xf3\x1e", "\x02\xb4\x01", "\x02\xf7\x1e", "\x02\x75\x05",
  "\x02\x57\x04", "\x02\x62\x31", "\x02\x2f\x26", "\x02\x82\x05",
  "\x02\x59\xff", "\x02\xd9\x05", "\x02\x39\xfb", "\x02\x39\xfb",
  "\x02\xd9\x05", "\x02\xf2\x05", "\x02\x1f\xfb", "\x02\x88\x30",
  "\x02\x89\x31", "\x02\xe8\x30", "\x02\x96\xff", "\x02\x5b\x31",
  "\x02\x87\x30", "\x02\xe7\x30", "\x02\x6e\xff", "\x02\xf3\x03",
  "\x02\x88\x31", "\x02\x87\x31", "\x02\x22\x0e", "\x02\x0d\x0e",
  "\x02\xb4\x24", "\x02\x7a\x03", "\x02\x45\x03", "\x02\xa6\x01",
  "\x02\x99\x1e", "\x02\xb8\x02", "\x02\xf9\x1e", "\x02\x8e\x02",
  "\x02\x86\x30", "\x02\x8c\x31", "\x02\xe6\x30", "\x02\x95\xff",
  "\x02\x60\x31", "\x02\x6b\x04", "\x02\x6d\x04", "\x02\x67\x04",
  "\x02\x69\x04", "\x02\x85\x30", "\x02\xe5\x30", "\x02\x6d\xff",
  "\x02\x8b\x31", "\x02\x8a\x31", "\x02\xdf\x09", "\x02\x5f\x09",
  "\x02\x7a\x00", "\x02\x66\x05", "\x02\x7a\x01", "\x02\x5b\x09",
  "\x02\x5b\x0a", "\x02\x38\x06", "\x02\xc6\xfe", "\x02\xc7\xfe",
  "\x02\x56\x30", "\x02\xc8\xfe", "\x02\x32\x06", "\x02\xb0\xfe",
  "\x02\xb6\x30", "\x02\x95\x05", "\x02\x94\x05", "\x02\x98\x05",
  "\x02\xd6\x05", "\x02\x36\xfb", "\x02\x36\xfb", "\x02\xd6\x05",
  "\x02\x17\x31", "\x02\x7e\x01", "\x02\xe9\x24", "\x02\x91\x1e",
  "\x02\x91\x02", "\x02\x7c\x01", "\x02\x7c\x01", "\x02\x93\x1e",
  "\x02\x37\x04", "\x02\x99\x04", "\x02\xdf\x04", "\x02\x5c\x30",
  "\x02\xbc\x30", "\x02\x30\x00", "\x02\x60\x06", "\x02\xe6\x09",
  "\x02\x66\x09", "\x02\xe6\x0a", "\x02\x66\x0a", "\x02\x60\x06",
  "\x02\x80\x20", "\x02\x10\xff", "\x02\x30\xf7", "\x02\xf0\x06",
  "\x02\x70\x20", "\x02\x50\x0e", "\x02\xff\xfe", "\x02\x0c\x20",
  "\x02\x0b\x20", "\x02\xb6\x03", "\x02\x13\x31", "\x02\x6a\x05",
  "\x02\xc2\x04", "\x02\x36\x04", "\x02\x97\x04", "\x02\xdd\x04",
  "\x02\x58\x30", "\x02\xb8\x30", "\x02\xae\x05", "\x02\x95\x1e",
  "\x02\x5a\xff", "\x02\x5e\x30", "\x02\xbe\x30", "\x02\xb5\x24",
  "\x02\x90\x02", "\x02\xb6\x01", "\x02\x5a\x30", "\x02\xba\x30",
  NULL
};


static const char *raw_unicode_translations[] = {
  "\x00\xa0\x20", "\x00\xad", "\x00\xc6\x41\x45", "\x00\xe6\x61\x65",
  "\x01\xf1\x44\x5a", "\x01\xf2\x44\x7a", "\x01\xf3\x64\x7a", "\x20\x00\x20",
  "\x20\x01\x20", "\x20\x02\x20", "\x20\x03\x20", "\x20\x04\x20",
  "\x20\x05\x20", "\x20\x06\x20", "\x20\x07\x20", "\x20\x08\x20",
  "\x20\x09\x20", "\x20\x0a\x20", "\x20\x0b\x20", "\x20\x10\x2d",
  "\x20\x11\x2d", "\x20\x12\x2d", "\x20\x13\x2d", "\x20\x14\x2d",
  "\x20\x18\x27", "\x20\x19\x27", "\x20\x1c\x22", "\x20\x1d\x22",
  "\x20\x26\x2e\x2e\x2e", "\x22\x12\x2d", "\x30\x00\x20", "\xfb\x00\x66\x66",
  "\xfb\x01\x66\x69", "\xfb\x02\x66\x6c", "\xfb\x03\x66\x66\x69",
  "\xfb\x04\x66\x66\x6c", "\xfb\x06\x73\x74", NULL
};


#if PY_MAJOR_VERSION >= 3

static int pycpdf_traverse(PyObject *m, visitproc visit, void *arg) {
  /* Py_VISIT(GETSTATE(m)->baseerror); */
  return 0;
}

static int pycpdf_clear(PyObject *m) {
  /* Py_CLEAR(GETSTATE(m)->error); */
  return 0;
}

static struct PyModuleDef moduledef = {
  PyModuleDef_HEAD_INIT,
  "pycpdf",
  pycpdf_doc,
  -1,
  pycpdf_methods,
  NULL,
  pycpdf_traverse,
  pycpdf_clear,
  NULL
};

#endif

#if PY_MAJOR_VERSION >= 3
PyMODINIT_FUNC PyInit_pycpdf(void) {
#else
PyMODINIT_FUNC initpycpdf(void) {
#endif
  PyObject *m = NULL;
  PyObject *othermod = NULL;
  const char **glyphname;
  const char **glyphvalue;
  PyObject *obj = NULL;
  PyObject *obj2 = NULL;
  int byteorder;
  const unsigned char *translation;

#if PY_MAJOR_VERSION >= 3
  if (!(m = PyModule_Create(&moduledef)))
    goto error;
#else
  if (!(m = Py_InitModule3("pycpdf", pycpdf_methods, pycpdf_doc)))
    goto error;
#endif

  if (!(othermod = PyImport_ImportModule("hashlib")))
    goto error;
  md5 = PyObject_GetAttrString(othermod, "md5");
  Py_DECREF(othermod);
  if (!md5)
    goto error;

  if (!(othermod = PyImport_ImportModule("zlib")))
    goto error;
  decompressobj = PyObject_GetAttrString(othermod, "decompressobj");
  Py_DECREF(othermod);
  if (!decompressobj)
    goto error;

  if (!(string_prev = PyUnicode_DecodeASCII("Prev", 4, "strict")))
    goto error;
  if (!(string_page = PyUnicode_DecodeASCII("Page", 4, "strict")))
    goto error;
  if (!(string_emptystring = PyUnicode_DecodeASCII("", 0, "strict")))
    goto error;
  if (!(string_newline = PyUnicode_DecodeASCII("\n", 1, "strict")))
    goto error;
  byteorder = -1;
  if (!(string_unknown = PyUnicode_DecodeUTF16("\xfe\xff", 2, "strict",
          &byteorder)))
    goto error;
  if (!(bytestr_emptystring = PyBytes_FromString("")))
    goto error;

  NameType.tp_base = &PyUnicode_Type;
  if (PyType_Ready(&NameType) < 0)
    goto error;
  OperatorType.tp_base = &PyUnicode_Type;
  if (PyType_Ready(&OperatorType) < 0)
    goto error;
  PDFStringType.tp_base = &PyUnicode_Type;
  if (PyType_Ready(&PDFStringType) < 0)
    goto error;
  ArrayType.tp_base = &PyList_Type;
  ArrayType.tp_traverse = PyList_Type.tp_traverse;
  ArrayType.tp_clear = PyList_Type.tp_clear;
  if (PyType_Ready(&ArrayType) < 0)
    goto error;
  if (PyType_Ready(&ArrayIteratorType) < 0)
    goto error;
  DictionaryType.tp_base = &PyDict_Type;
  DictionaryType.tp_traverse = PyDict_Type.tp_traverse;
  DictionaryType.tp_clear = PyDict_Type.tp_clear;
  if (PyType_Ready(&DictionaryType) < 0)
    goto error;
  StreamObjectType.tp_base = &DictionaryType;
  if (PyType_Ready(&StreamObjectType) < 0)
    goto error;
  PageType.tp_base = &DictionaryType;
  if (PyType_Ready(&PageType) < 0)
    goto error;
  OffsetType.tp_base = &PyLong_Type;
  if (PyType_Ready(&OffsetType) < 0)
    goto error;
  if (PyType_Ready(&StreamReferenceType) < 0)
    goto error;
  if (PyType_Ready(&IndirectObjectType) < 0)
    goto error;
  if (PyType_Ready(&PDFType) < 0)
    goto error;

  if (!(name_inlineimage = PyObject_CallFunction((PyObject *)&NameType,
          "s", "InlineImage")))
    goto error;

  if (!baseerror)
    if (!(baseerror = PyErr_NewException("pycpdf.Error", NULL, NULL)))
      goto error;
  if (!pdferror)
    if (!(pdferror = PyErr_NewException("pycpdf.FormatError", baseerror,
            NULL)))
      goto error;
  if (!notsupportederror)
    if (!(notsupportederror = PyErr_NewException("pycpdf.NotSupportedError",
            baseerror, NULL)))
      goto error;
  if (!badpassworderror)
    if (!(badpassworderror = PyErr_NewException("pycpdf.BadPasswordError",
            baseerror, NULL)))
      goto error;
  if (!(encodings = PyDict_New()))
    goto error;
  byteorder = -1;
  if (!(pdfdocencoding = PyUnicode_DecodeUTF16(raw_pdfdocencoding, 512,
          "strict", &byteorder)) || PyMapping_SetItemString(encodings,
          "PDFDocEncoding", pdfdocencoding) < 0)
    goto error;
  byteorder = -1;
  if (!(obj = PyUnicode_DecodeUTF16(raw_standardencoding, 512,
          "strict", &byteorder)) || PyMapping_SetItemString(encodings,
          "StandardEncoding", obj) < 0)
    goto error;
  Py_DECREF(obj);
  byteorder = -1;
  if (!(obj = PyUnicode_DecodeUTF16(raw_macromanencoding, 512,
          "strict", &byteorder)) || PyMapping_SetItemString(encodings,
          "MacRomanEncoding", obj) < 0)
    goto error;
  Py_DECREF(obj);
  byteorder = -1;
  if (!(obj = PyUnicode_DecodeUTF16(raw_winansiencoding, 512,
          "strict", &byteorder)) || PyMapping_SetItemString(encodings,
          "WinAnsiEncoding", obj) < 0)
    goto error;
  Py_DECREF(obj);
  byteorder = -1;
  if (!(obj = PyUnicode_DecodeUTF16(raw_macexpertencoding, 512,
          "strict", &byteorder)) || PyMapping_SetItemString(encodings,
          "MacExpertEncoding", obj) < 0)
    goto error;
  Py_DECREF(obj);
  Py_INCREF(encodings);
  PyModule_AddObject(m, "encodings", encodings);
  if (!(glyphlist = PyDict_New()))
    goto error;
  glyphname = glyph_names;
  glyphvalue = glyph_values;
  while (*glyphname) {
    byteorder = -1;
    if (!(obj = PyUnicode_DecodeUTF16((*glyphvalue) + 1, (*glyphvalue)[0],
            "strict", &byteorder)))
      goto error;
    if (PyMapping_SetItemString(glyphlist, (char *)*glyphname, obj) < 0)
      goto error;
    Py_DECREF(obj);
    glyphname++;
    glyphvalue++;
  }
  Py_INCREF(glyphlist);
  PyModule_AddObject(m, "glyphlist", glyphlist);
  if (!(unicode_translations = PyDict_New()))
    goto error;
  for (glyphname = raw_unicode_translations; *glyphname; glyphname++) {
    translation = (const unsigned char *)*glyphname;
    byteorder = 1;
    if (!(obj = PyLong_FromLong(translation[0] << 8 | translation[1])))
      goto error;
    if (!translation[2]) {
      Py_INCREF(Py_None);
      obj2 = Py_None;
    } else {
      if (!(obj2 = PyUnicode_DecodeASCII((char *)translation + 2,
              strlen((char *)translation + 2), "strict")))
        goto error;
    }
    if (PyObject_SetItem(unicode_translations, obj, obj2) < 0)
      goto error;
    Py_DECREF(obj2);
    Py_DECREF(obj);
  }
  Py_INCREF(unicode_translations);
  PyModule_AddObject(m, "unicode_translations", unicode_translations);
  Py_INCREF(baseerror);
  PyModule_AddObject(m, "Error", baseerror);
  Py_INCREF(pdferror);
  PyModule_AddObject(m, "FormatError", pdferror);
  Py_INCREF(notsupportederror);
  PyModule_AddObject(m, "NotSupportedError", notsupportederror);
  Py_INCREF(badpassworderror);
  PyModule_AddObject(m, "BadPasswordError", badpassworderror);
  Py_INCREF(&NameType);
  PyModule_AddObject(m, "Name", (PyObject *)&NameType);
  Py_INCREF(&OperatorType);
  PyModule_AddObject(m, "Operator", (PyObject *)&OperatorType);
  Py_INCREF(&PDFStringType);
  PyModule_AddObject(m, "PDFString", (PyObject *)&PDFStringType);
  Py_INCREF(&ArrayType);
  PyModule_AddObject(m, "Array", (PyObject *)&ArrayType);
  Py_INCREF(&DictionaryType);
  PyModule_AddObject(m, "Dictionary", (PyObject *)&DictionaryType);
  Py_INCREF(&StreamObjectType);
  PyModule_AddObject(m, "StreamObject", (PyObject *)&StreamObjectType);
  Py_INCREF(&PageType);
  PyModule_AddObject(m, "Page", (PyObject *)&PageType);
  Py_INCREF(&OffsetType);
  PyModule_AddObject(m, "Offset", (PyObject *)&OffsetType);
  Py_INCREF(&StreamReferenceType);
  PyModule_AddObject(m, "StreamReference", (PyObject *)&StreamReferenceType);
  Py_INCREF(&IndirectObjectType);
  PyModule_AddObject(m, "IndirectObject", (PyObject *)&IndirectObjectType);
  Py_INCREF(&PDFType);
  PyModule_AddObject(m, "PDF", (PyObject *)&PDFType);
#if PY_MAJOR_VERSION >= 3
  return m;
#else
  return;
#endif
error:
  Py_XDECREF(othermod);
  Py_XDECREF(obj);
  Py_XDECREF(obj2);
#if PY_MAJOR_VERSION >= 3
  return NULL;
#else
  return;
#endif
}
