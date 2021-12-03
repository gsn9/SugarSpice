%module pyspiceql

%include std_string.i
%include std_vector.i

#include <nlohmann/json.hpp>

%typemap(in) nlohmann::json {
  if (PyDict_Check($input)) {
    PyObject* module = PyImport_ImportModule("json");
    PyObject* jsonDumps = PyUnicode_FromString("dumps");
    PyObject* pythonJsonString = PyObject_CallMethodObjArgs(module, jsonDumps, $input, NULL);
    $1 = nlohmann::json::parse(PyUnicode_AsUTF8(pythonJsonString));
  }
  else {
    PyErr_SetString(PyExc_TypeError, "not a dictionary");
    SWIG_fail;
  }
}

%typemap(out) nlohmann::json {
  PyObject* module = PyImport_ImportModule("json");
  PyObject* jsonLoads = PyUnicode_FromString("loads");
  std::string jsonString = ($1).dump();
  PyObject* pythonJsonString = PyUnicode_DecodeUTF8(jsonString.c_str(), jsonString.size(), NULL);
  $result = PyObject_CallMethodObjArgs(module, jsonLoads, pythonJsonString, NULL);
}

%template(_string_list) std::vector< std::string >;

%exception {
  try {
    $action
  } catch (std::exception const& e) {
    SWIG_exception(SWIG_RuntimeError, (std::string("std::exception: ") + e.what()).c_str());
  } catch (...) {
    SWIG_exception(SWIG_UnknownError, "Unknown error");
  }
}

%include "config.i"
%include "io.i"
%include "query.i"
%include "spice_types.i"
%include "utils.i"