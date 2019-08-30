#include <Python.h>
#include "htmldoc.h"

typedef enum {
    DATA_TYPE_TEXT = 0,
    DATA_TYPE_ARRAY
} data_type_t;

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

static int read_html(char *html, size_t len, tree_t **document) {
    _htmlPPI = 72.0f * _htmlBrowserWidth / (PageWidth - PageLeft - PageRight);
    tree_t *file = htmlAddTree(NULL, MARKUP_FILE, NULL);
    if (!file) { PyErr_SetString(PyExc_TypeError, "!file"); return 0; }
    htmlSetVariable(file, (uchar *)"_HD_FILENAME", (uchar *)"");
    htmlSetVariable(file, (uchar *)"_HD_BASE", (uchar *)".");
    FILE *in = fmemopen(html, len, "rb");
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

static PyObject *htmldoc(PyObject *data, const char *file, data_type_t data_type, input_type_t input_type, output_type_t output_type) {
    PyObject *bytes = PyBytes_FromString(""), *iterator, *item;
    char *input_data;
    Py_ssize_t input_len;
    htmlSetCharSet("utf-8");
    tree_t *document = NULL;
    switch (input_type) {
        case INPUT_TYPE_FILE: {
            if (PyBytes_Check(data)) {
                if (PyBytes_AsStringAndSize(data, &input_data, &input_len)) { PyErr_SetString(PyExc_TypeError, "PyBytes_AsStringAndSize"); Py_RETURN_NONE; }
                if (!read_fileurl(input_data, &document, Path)) Py_RETURN_NONE;
            } else {
                if (!(iterator = PyObject_GetIter(data))) { PyErr_SetString(PyExc_TypeError, "!iterator"); Py_RETURN_NONE; }
                while ((item = PyIter_Next(iterator))) {
                    if (PyBytes_AsStringAndSize(item, &input_data, &input_len)) { PyErr_SetString(PyExc_TypeError, "PyBytes_AsStringAndSize"); Py_RETURN_NONE; }
                    if (!read_fileurl(input_data, &document, Path)) Py_RETURN_NONE;
                    Py_DECREF(item);
                }
                Py_DECREF(iterator);
            }
        } break;
        case INPUT_TYPE_HTML: {
            if (PyBytes_Check(data)) {
                if (PyBytes_AsStringAndSize(data, &input_data, &input_len)) { PyErr_SetString(PyExc_TypeError, "PyBytes_AsStringAndSize"); Py_RETURN_NONE; }
                if (!read_html(input_data, input_len, &document)) Py_RETURN_NONE;
            } else {
                if (!(iterator = PyObject_GetIter(data))) { PyErr_SetString(PyExc_TypeError, "!iterator"); Py_RETURN_NONE; }
                while ((item = PyIter_Next(iterator))) {
                    if (PyBytes_AsStringAndSize(item, &input_data, &input_len)) { PyErr_SetString(PyExc_TypeError, "PyBytes_AsStringAndSize"); Py_RETURN_NONE; }
                    if (!read_html(input_data, input_len, &document)) Py_RETURN_NONE;
                    Py_DECREF(item);
                }
                Py_DECREF(iterator);
            }
        } break;
        case INPUT_TYPE_URL: {
            if (PyBytes_Check(data)) {
                if (PyBytes_AsStringAndSize(data, &input_data, &input_len)) { PyErr_SetString(PyExc_TypeError, "PyBytes_AsStringAndSize"); Py_RETURN_NONE; }
                if (!read_fileurl(input_data, &document, NULL)) Py_RETURN_NONE;
            } else {
                if (!(iterator = PyObject_GetIter(data))) { PyErr_SetString(PyExc_TypeError, "!iterator"); Py_RETURN_NONE; }
                while ((item = PyIter_Next(iterator))) {
                    if (PyBytes_AsStringAndSize(data, &input_data, &input_len)) { PyErr_SetString(PyExc_TypeError, "PyBytes_AsStringAndSize"); Py_RETURN_NONE; }
                    if (!read_fileurl(input_data, &document, NULL)) Py_RETURN_NONE;
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
    FILE *out;
    char *output_data = NULL;
    size_t output_len = 0;
    if (!file) {
        if (!(out = open_memstream(&output_data, &output_len))) { PyErr_SetString(PyExc_TypeError, "!out"); Py_RETURN_NONE; }
    } else {
        if (!(out = fopen(file, "wb"))) { PyErr_SetString(PyExc_TypeError, "!out"); Py_RETURN_NONE; }
    }
    pspdf_export_out(document, NULL, out);
    if (document) htmlDeleteTree(document);
    file_cleanup();
    image_flush_cache();
    if (!file) {
        bytes = PyBytes_FromStringAndSize(output_data, (Py_ssize_t)output_len);
        free(output_data);
        return bytes;
    } else Py_RETURN_TRUE;
}

PyObject *file2pdf(PyObject *data, const char *file) { return htmldoc(data, file, DATA_TYPE_TEXT, INPUT_TYPE_FILE, OUTPUT_TYPE_PDF); }
PyObject *file2ps(PyObject *data, const char *file) { return htmldoc(data, file, DATA_TYPE_TEXT, INPUT_TYPE_FILE, OUTPUT_TYPE_PS); }
PyObject *html2pdf(PyObject *data, const char *file) { return htmldoc(data, file, DATA_TYPE_TEXT, INPUT_TYPE_HTML, OUTPUT_TYPE_PDF); }
PyObject *html2ps(PyObject *data, const char *file) { return htmldoc(data, file, DATA_TYPE_TEXT, INPUT_TYPE_HTML, OUTPUT_TYPE_PS); }
PyObject *url2pdf(PyObject *data, const char *file) { return htmldoc(data, file, DATA_TYPE_TEXT, INPUT_TYPE_URL, OUTPUT_TYPE_PDF); }
PyObject *url2ps(PyObject *data, const char *file) { return htmldoc(data, file, DATA_TYPE_TEXT, INPUT_TYPE_URL, OUTPUT_TYPE_PS); }
