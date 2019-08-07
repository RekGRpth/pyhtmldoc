%module pyhtmldoc
%{
extern PyObject *htmldoc(PyObject *data, const char *input_type, const char *output_type, const char *options, const char *range);
%}

extern PyObject *htmldoc(PyObject *data, const char *input_type, const char *output_type, const char *options, const char *range);
