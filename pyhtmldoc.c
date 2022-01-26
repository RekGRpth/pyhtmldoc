#include <Python.h>
#include "htmldoc.h"

typedef enum {
    INPUT_TYPE_FILE = 0,
    INPUT_TYPE_HTML,
    INPUT_TYPE_URL
} input_type_t;

typedef enum {
    OUTPUT_TYPE_PDF = 0,
    OUTPUT_TYPE_PS
} output_type_t;

static int read_fileurl(const char *fileurl, tree_t **document, const char *path) {
    _htmlPPI = 72.0f * _htmlBrowserWidth / (PageWidth - PageLeft - PageRight);
    tree_t *file = htmlAddTree(NULL, MARKUP_FILE, NULL);
    if (!file) { PyErr_SetString(PyExc_TypeError, "!file"); return 0; }
    htmlSetVariable(file, (uchar *)"_HD_URL", (uchar *)fileurl);
    htmlSetVariable(file, (uchar *)"_HD_FILENAME", (uchar *)file_basename(fileurl));
    const char *realname = file_find(path, fileurl);
    if (!realname) { PyErr_SetString(PyExc_TypeError, "!realname"); return 0; }
    const char *base = file_directory(fileurl);
    if (!base) { PyErr_SetString(PyExc_TypeError, "!base"); return 0; }
    htmlSetVariable(file, (uchar *)"_HD_BASE", (uchar *)base);
    FILE *in = fopen(realname, "rb");
    if (!in) { PyErr_SetString(PyExc_TypeError, "!in"); return 0; }
    htmlReadFile2(file, in, base);
    fclose(in);
    if (*document == NULL) *document = file; else {
        while ((*document)->next != NULL) *document = (*document)->next;
        (*document)->next = file;
        file->prev = *document;
    }
    return 1;
}

static int read_html(const char *html, size_t len, tree_t **document) {
    _htmlPPI = 72.0f * _htmlBrowserWidth / (PageWidth - PageLeft - PageRight);
    tree_t *file = htmlAddTree(NULL, MARKUP_FILE, NULL);
    if (!file) { PyErr_SetString(PyExc_TypeError, "!file"); return 0; }
    htmlSetVariable(file, (uchar *)"_HD_FILENAME", (uchar *)"");
    htmlSetVariable(file, (uchar *)"_HD_BASE", (uchar *)".");
    FILE *in = fmemopen((void *)html, len, "rb");
    if (!in) { PyErr_SetString(PyExc_TypeError, "!in"); return 0; }
    htmlReadFile2(file, in, ".");
    fclose(in);
    if (*document == NULL) *document = file; else {
        while ((*document)->next != NULL) *document = (*document)->next;
        (*document)->next = file;
        file->prev = *document;
    }
    return 1;
}

#if PY_VERSION_HEX < 0x03000000
const char *PyUnicode_AsUTF8AndSize(PyObject *unicode, Py_ssize_t *psize) {
    if (!PyUnicode_Check(unicode)) { PyErr_BadArgument(); return NULL; }
    PyObject *utf8 = PyUnicode_AsUTF8String(unicode);
    if (!utf8) { PyErr_BadArgument(); return NULL; }
    const char *data = PyString_AsString(utf8);
    if (psize) *psize = PyString_GET_SIZE(utf8);
    return data;
}

const char *PyUnicode_AsUTF8(PyObject *unicode) {
    return PyUnicode_AsUTF8AndSize(unicode, NULL);
}
#endif

static PyObject *htmldoc(PyObject *data, const char *file, input_type_t input_type, output_type_t output_type) {
    PyObject *bytes = NULL, *iterator, *item;
    const char *input_data;
    Py_ssize_t input_len;
    tree_t *document = NULL;
    if (!_htmlInitialized) htmlSetCharSet("utf-8");
    switch (input_type) {
        case INPUT_TYPE_FILE: {
            if (PyUnicode_Check(data)) {
                if (!(input_data = PyUnicode_AsUTF8AndSize(data, &input_len))) { PyErr_SetString(PyExc_TypeError, "!PyUnicode_AsUTF8AndSize"); goto ret; }
                if (!read_fileurl(input_data, &document, Path)) goto ret;
            } else {
                if (!(iterator = PyObject_GetIter(data))) { PyErr_SetString(PyExc_TypeError, "!iterator"); goto ret; }
                while ((item = PyIter_Next(iterator))) {
                    if (!(input_data = PyUnicode_AsUTF8AndSize(item, &input_len))) { PyErr_SetString(PyExc_TypeError, "!PyUnicode_AsUTF8AndSize"); goto dec; }
                    if (!read_fileurl(input_data, &document, Path)) goto dec;
                    Py_DECREF(item);
                }
                Py_DECREF(iterator);
            }
        } break;
        case INPUT_TYPE_HTML: {
            if (PyUnicode_Check(data)) {
                if (!(input_data = PyUnicode_AsUTF8AndSize(data, &input_len))) { PyErr_SetString(PyExc_TypeError, "!PyUnicode_AsUTF8AndSize"); goto ret; }
                if (!read_html(input_data, input_len, &document)) goto ret;
            } else {
                if (!(iterator = PyObject_GetIter(data))) { PyErr_SetString(PyExc_TypeError, "!iterator"); goto ret; }
                while ((item = PyIter_Next(iterator))) {
                    if (!(input_data = PyUnicode_AsUTF8AndSize(item, &input_len))) { PyErr_SetString(PyExc_TypeError, "!PyUnicode_AsUTF8AndSize"); goto dec; }
                    if (!read_html(input_data, input_len, &document)) goto dec;
                    Py_DECREF(item);
                }
                Py_DECREF(iterator);
            }
        } break;
        case INPUT_TYPE_URL: {
            if (PyUnicode_Check(data)) {
                if (!(input_data = PyUnicode_AsUTF8AndSize(data, &input_len))) { PyErr_SetString(PyExc_TypeError, "!PyUnicode_AsUTF8AndSize"); goto ret; }
                if (!read_fileurl(input_data, &document, NULL)) goto ret;
            } else {
                if (!(iterator = PyObject_GetIter(data))) { PyErr_SetString(PyExc_TypeError, "!iterator"); goto ret; }
                while ((item = PyIter_Next(iterator))) {
                    if (!(input_data = PyUnicode_AsUTF8AndSize(item, &input_len))) { PyErr_SetString(PyExc_TypeError, "!PyUnicode_AsUTF8AndSize"); goto dec; }
                    if (!read_fileurl(input_data, &document, NULL)) goto dec;
                    Py_DECREF(item);
                }
                Py_DECREF(iterator);
            }
        } break;
    }
    while (document && document->prev) document = document->prev;
    htmlFixLinks(document, document, 0);
    switch (output_type) {
        case OUTPUT_TYPE_PDF: PSLevel = 0; break;
        case OUTPUT_TYPE_PS: PSLevel = 3; break;
    }
    char *output_data = NULL;
    FILE *out;
    size_t output_len = 0;
    if (file) {
        if (!(out = fopen(file, "wb"))) { PyErr_SetString(PyExc_TypeError, "!out"); goto ret; }
    } else {
        if (!(out = open_memstream(&output_data, &output_len))) { PyErr_SetString(PyExc_TypeError, "!out"); goto ret; }
    }
    pspdf_export_out(document, NULL, out);
    bytes = file ? Py_True : PyBytes_FromStringAndSize(output_data, (Py_ssize_t)output_len);
    if (output_data) free(output_data);
ret:
    if (document) htmlDeleteTree(document);
    file_cleanup();
    image_flush_cache();
    return bytes;
dec:
    Py_DECREF(item);
    Py_DECREF(iterator);
    goto ret;
}

PyObject *file2pdf(PyObject *data) { return htmldoc(data, NULL, INPUT_TYPE_FILE, OUTPUT_TYPE_PDF); }
PyObject *file2ps(PyObject *data) { return htmldoc(data, NULL, INPUT_TYPE_FILE, OUTPUT_TYPE_PS); }
PyObject *html2pdf(PyObject *data) { return htmldoc(data, NULL, INPUT_TYPE_HTML, OUTPUT_TYPE_PDF); }
PyObject *html2ps(PyObject *data) { return htmldoc(data, NULL, INPUT_TYPE_HTML, OUTPUT_TYPE_PS); }
PyObject *url2pdf(PyObject *data) { return htmldoc(data, NULL, INPUT_TYPE_URL, OUTPUT_TYPE_PDF); }
PyObject *url2ps(PyObject *data) { return htmldoc(data, NULL, INPUT_TYPE_URL, OUTPUT_TYPE_PS); }

PyObject *file2pdf_file(PyObject *data, const char *file) { return htmldoc(data, file, INPUT_TYPE_FILE, OUTPUT_TYPE_PDF); }
PyObject *file2ps_file(PyObject *data, const char *file) { return htmldoc(data, file, INPUT_TYPE_FILE, OUTPUT_TYPE_PS); }
PyObject *html2pdf_file(PyObject *data, const char *file) { return htmldoc(data, file, INPUT_TYPE_HTML, OUTPUT_TYPE_PDF); }
PyObject *html2ps_file(PyObject *data, const char *file) { return htmldoc(data, file, INPUT_TYPE_HTML, OUTPUT_TYPE_PS); }
PyObject *url2pdf_file(PyObject *data, const char *file) { return htmldoc(data, file, INPUT_TYPE_URL, OUTPUT_TYPE_PDF); }
PyObject *url2ps_file(PyObject *data, const char *file) { return htmldoc(data, file, INPUT_TYPE_URL, OUTPUT_TYPE_PS); }
