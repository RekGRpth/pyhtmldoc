#include <Python.h>
#include "htmldoc.h"

typedef enum {
    INPUT_TYPE_HTML = 0,
    INPUT_TYPE_URL
} input_type_t;

typedef enum {
    OUTPUT_TYPE_PDF = 0,
    OUTPUT_TYPE_PS
} output_type_t;

static PyObject *htmldoc(PyObject *data, input_type_t input_type, output_type_t output_type) {
    PyObject *bytes = PyBytes_FromString("");
    char *input_data;
    Py_ssize_t input_len;
    if (PyBytes_AsStringAndSize(data, &input_data, &input_len)) goto ret;
    _htmlPPI = 72.0f * _htmlBrowserWidth / (PageWidth - PageLeft - PageRight);
    htmlSetCharSet("utf-8");
    tree_t *document = htmlAddTree(NULL, MARKUP_FILE, NULL);
    if (!document) goto ret;
    if (input_type == INPUT_TYPE_HTML) {
        FILE *in = fmemopen(input_data, input_len, "rb");
        if (!in) goto htmlDeleteTree;
        htmlSetVariable(document, (uchar *)"_HD_FILENAME", (uchar *)"");
        htmlSetVariable(document, (uchar *)"_HD_BASE", (uchar *)".");
        htmlReadFile2(document, in, ".");
        fclose(in);
    } else if (input_type == INPUT_TYPE_URL) {
        const char *realname = file_find(NULL, input_data);
        if (!realname) goto htmlDeleteTree;
        FILE *in = fopen(realname, "rb");
        if (!in) goto htmlDeleteTree;
        char *base = strdup(file_directory(input_data));
        if (!base) { fclose(in); goto htmlDeleteTree; }
        htmlSetVariable(document, (uchar *)"_HD_URL", (uchar *)input_data);
        htmlSetVariable(document, (uchar *)"_HD_FILENAME", (uchar *)file_basename(input_data));
        htmlSetVariable(document, (uchar *)"_HD_BASE", (uchar *)base);
        htmlReadFile2(document, in, base);
        fclose(in);
        free(base);
    }
    htmlFixLinks(document, document, 0);
    if (output_type == OUTPUT_TYPE_PDF) {
        PSLevel = 0;
    } else if (output_type == OUTPUT_TYPE_PS) {
        PSLevel = 3;
    }
    char *output_data = NULL;
    size_t output_len = 0;
    FILE *out = open_memstream(&output_data, &output_len);
    if (!out) goto htmlDeleteTree;
    pspdf_export_out(document, NULL, out);
    bytes = PyBytes_FromStringAndSize(output_data, (Py_ssize_t)output_len);
htmlDeleteTree:
    htmlDeleteTree(document);
    file_cleanup();
    image_flush_cache();
ret:
    return bytes;
}

PyObject *html2pdf(PyObject *data) {
    return htmldoc(data, INPUT_TYPE_HTML, OUTPUT_TYPE_PDF);
}

PyObject *html2ps(PyObject *data) {
    return htmldoc(data, INPUT_TYPE_HTML, OUTPUT_TYPE_PS);
}

PyObject *url2pdf(PyObject *data) {
    return htmldoc(data, INPUT_TYPE_URL, OUTPUT_TYPE_PDF);
}

PyObject *url2ps(PyObject *data) {
    return htmldoc(data, INPUT_TYPE_URL, OUTPUT_TYPE_PS);
}
