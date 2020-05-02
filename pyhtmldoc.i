%module pyhtmldoc
%{
extern PyObject *file2pdf(PyObject *data, const char *file);
extern PyObject *file2ps(PyObject *data, const char *file);
extern PyObject *html2pdf(PyObject *data, const char *file);
extern PyObject *html2ps(PyObject *data, const char *file);
extern PyObject *url2pdf(PyObject *data, const char *file);
extern PyObject *url2ps(PyObject *data, const char *file);
%}

extern PyObject *file2pdf(PyObject *data, const char *file);
extern PyObject *file2ps(PyObject *data, const char *file);
extern PyObject *html2pdf(PyObject *data, const char *file);
extern PyObject *html2ps(PyObject *data, const char *file);
extern PyObject *url2pdf(PyObject *data, const char *file);
extern PyObject *url2ps(PyObject *data, const char *file);
