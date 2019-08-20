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
    tree_t *file = htmlAddTree(NULL, MARKUP_FILE, NULL);
    if (!file) return 0;
    htmlSetVariable(file, (uchar *)"_HD_URL", (uchar *)fileurl);
    htmlSetVariable(file, (uchar *)"_HD_FILENAME", (uchar *)file_basename(fileurl));
    const char *realname = file_find(path, fileurl);
    if (!realname) return 0;
    const char *base = file_directory(fileurl);
    if (!base) return 0;
    htmlSetVariable(file, (uchar *)"_HD_BASE", (uchar *)base);
    FILE *in = fopen(realname, "rb");
    if (!in) return 0;
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
    tree_t *file = htmlAddTree(NULL, MARKUP_FILE, NULL);
    if (!file) return 0;
    htmlSetVariable(file, (uchar *)"_HD_FILENAME", (uchar *)"");
    htmlSetVariable(file, (uchar *)"_HD_BASE", (uchar *)".");
    FILE *in = fmemopen(html, len, "rb");
    if (!in) return 0;
    htmlReadFile2(file, in, ".");
    fclose(in);
    if (*document == NULL) *document = file; else {
        while ((*document)->next != NULL) *document = (*document)->next;
        (*document)->next = file;
        file->prev = *document;
    }
    return 1;
}

static PyObject *htmldoc(PyObject *data, data_type_t data_type, input_type_t input_type, output_type_t output_type) {
    PyObject *bytes = PyBytes_FromString("");
    char *input_data;
    Py_ssize_t input_len;
    if (PyBytes_AsStringAndSize(data, &input_data, &input_len)) goto ret;
    _htmlPPI = 72.0f * _htmlBrowserWidth / (PageWidth - PageLeft - PageRight);
    htmlSetCharSet("utf-8");
    tree_t *document = NULL;
    switch (input_type) {
        case INPUT_TYPE_FILE: if (!read_fileurl(input_data, &document, Path)) goto htmlDeleteTree; break;
        case INPUT_TYPE_HTML: if (!read_html(input_data, input_len, &document)) goto htmlDeleteTree; break;
        case INPUT_TYPE_URL: if (!read_fileurl(input_data, &document, NULL)) goto htmlDeleteTree; break;
    }
    htmlFixLinks(document, document, 0);
    switch (output_type) {
        case OUTPUT_TYPE_PDF: PSLevel = 0; break;
        case OUTPUT_TYPE_PS: PSLevel = 3; break;
    }
    char *output_data = NULL;
    size_t output_len = 0;
    FILE *out = open_memstream(&output_data, &output_len);
    if (!out) goto htmlDeleteTree;
    pspdf_export_out(document, NULL, out);
    bytes = PyBytes_FromStringAndSize(output_data, (Py_ssize_t)output_len);
    free(output_data);
htmlDeleteTree:
    if (document) htmlDeleteTree(document);
    file_cleanup();
    image_flush_cache();
ret:
    return bytes;
}

PyObject *file2pdf(PyObject *data) { return htmldoc(data, DATA_TYPE_TEXT, INPUT_TYPE_FILE, OUTPUT_TYPE_PDF); }
PyObject *file2ps(PyObject *data) { return htmldoc(data, DATA_TYPE_TEXT, INPUT_TYPE_FILE, OUTPUT_TYPE_PS); }
PyObject *html2pdf(PyObject *data) { return htmldoc(data, DATA_TYPE_TEXT, INPUT_TYPE_HTML, OUTPUT_TYPE_PDF); }
PyObject *html2ps(PyObject *data) { return htmldoc(data, DATA_TYPE_TEXT, INPUT_TYPE_HTML, OUTPUT_TYPE_PS); }
PyObject *url2pdf(PyObject *data) { return htmldoc(data, DATA_TYPE_TEXT, INPUT_TYPE_URL, OUTPUT_TYPE_PDF); }
PyObject *url2ps(PyObject *data) { return htmldoc(data, DATA_TYPE_TEXT, INPUT_TYPE_URL, OUTPUT_TYPE_PS); }
