%module pyhtmldoc
%{
extern PyObject *file2pdf(PyObject *data);
extern PyObject *file2ps(PyObject *data);
extern PyObject *html2pdf(PyObject *data);
extern PyObject *html2ps(PyObject *data);
extern PyObject *url2pdf(PyObject *data);
extern PyObject *url2ps(PyObject *data);
%}

extern PyObject *file2pdf(PyObject *data);
extern PyObject *file2ps(PyObject *data);
extern PyObject *html2pdf(PyObject *data);
extern PyObject *html2ps(PyObject *data);
extern PyObject *url2pdf(PyObject *data);
extern PyObject *url2ps(PyObject *data);
