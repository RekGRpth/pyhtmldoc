#include <Python.h>
#include "htmldoc.h"

PyObject *htmldoc(PyObject *data) {
    PyObject *bytes = PyBytes_FromString("");
    char *input_data;
    Py_ssize_t input_len;
    if (PyBytes_AsStringAndSize(data, &input_data, &input_len)) goto ret;
    FILE *in = fmemopen(input_data, input_len, "rb");
    if (!in) goto ret;
    char *output_data = NULL;
    size_t output_len = 0;
    FILE *out = open_memstream(&output_data, &output_len);
    if (!out) goto fclose;
    set_out(out);
    htmlSetCharSet("utf-8");
    tree_t *document = htmlAddTree(NULL, MARKUP_FILE, NULL);
    if (!document) goto fclose;
    htmlSetVariable(document, (uchar *)"_HD_FILENAME", (uchar *)"");
    htmlSetVariable(document, (uchar *)"_HD_BASE", (uchar *)".");
    htmlReadFile(document, in, ".");
    htmlFixLinks(document, document, 0);
    pspdf_export(document, NULL);
    htmlDeleteTree(document);
    file_cleanup();
    image_flush_cache();
fclose:
    fclose(in);
    bytes = PyBytes_FromStringAndSize(output_data, (Py_ssize_t)output_len);
ret:
    return bytes;
}
