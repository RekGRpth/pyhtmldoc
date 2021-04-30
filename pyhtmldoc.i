%module pyhtmldoc
%{
extern PyObject *file2pdf(PyObject *data);
extern PyObject *file2ps(PyObject *data);
extern PyObject *html2pdf(PyObject *data);
extern PyObject *html2ps(PyObject *data);
extern PyObject *url2pdf(PyObject *data);
extern PyObject *url2ps(PyObject *data);

extern PyObject *file2pdf_file(PyObject *data, const char *file);
extern PyObject *file2ps_file(PyObject *data, const char *file);
extern PyObject *html2pdf_file(PyObject *data, const char *file);
extern PyObject *html2ps_file(PyObject *data, const char *file);
extern PyObject *url2pdf_file(PyObject *data, const char *file);
extern PyObject *url2ps_file(PyObject *data, const char *file);
%}

extern PyObject *file2pdf(PyObject *data);
extern PyObject *file2ps(PyObject *data);
extern PyObject *html2pdf(PyObject *data);
extern PyObject *html2ps(PyObject *data);
extern PyObject *url2pdf(PyObject *data);
extern PyObject *url2ps(PyObject *data);

extern PyObject *file2pdf_file(PyObject *data, const char *file);
extern PyObject *file2ps_file(PyObject *data, const char *file);
extern PyObject *html2pdf_file(PyObject *data, const char *file);
extern PyObject *html2ps_file(PyObject *data, const char *file);
extern PyObject *url2pdf_file(PyObject *data, const char *file);
extern PyObject *url2ps_file(PyObject *data, const char *file);
