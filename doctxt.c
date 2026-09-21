#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zip.h>
#include <libxml/parser.h>

#include "util.h"

char *
readzip(const char *path, const char *filename, size_t *len)
{
	zip_t *zf;
	zip_file_t *file;
	struct zip_stat st;
	zip_error_t error;
	zip_uint64_t size, off;
	zip_int64_t n;
	int err = 0;
	char *data;

	if ((zf = zip_open(path, 0, &err)) == NULL) {
		zip_error_init_with_code(&error, err);
		die("Unable to extract zip %s: %s", path, zip_error_strerror(&error));
	}

	if (zip_stat(zf, filename, 0, &st) != 0) {
		zip_close(zf);
		die("Unable to stat %s in zip", filename);
	}

	size = st.size;
	if (size > INT_MAX) {
		zip_close(zf);
		die("File %s too large in %s", filename, path);
	}

	if ((file = zip_fopen(zf, filename, 0)) == NULL) {
		zip_close(zf);
		die("%s not found in %s", filename, path);
	}

	data = ecalloc(sizeof(char), size + 1);

	for (off = 0; off < size; off += n) {
		if ((n = zip_fread(file, data + off, size - off)) <= 0) {
			zip_fclose(file);
			zip_close(zf);
			free(data);
			die("Unable to read %s in %s", filename, path);
		}
	}

	zip_fclose(file);
	zip_close(zf);

	data[size] = '\0'; /* fixes bug where sometimes file doesnt end in '\0' */
	*len = size;

	return data;
}

static void
walk(xmlNode *node, FILE *outfile)
{
	xmlNode *n, *next;
	xmlChar *text;

	for (n = node; n; n = n->next) {
		if (n->type != XML_ELEMENT_NODE)
			continue;
		if (xmlStrEqual(n->name, (const xmlChar *)"pPr") ||
		    xmlStrEqual(n->name, (const xmlChar *)"rPr") ||
		    xmlStrEqual(n->name, (const xmlChar *)"sectPr") ||
		    xmlStrEqual(n->name, (const xmlChar *)"tblPr") ||
		    xmlStrEqual(n->name, (const xmlChar *)"tcPr") ||
		    xmlStrEqual(n->name, (const xmlChar *)"trPr"))
			continue;
		if (xmlStrEqual(n->name, (const xmlChar *)"t")) {
			if ((text = xmlNodeGetContent(n)) != NULL) {
				fputs((const char *)text, outfile);
				xmlFree(text);
			}
		} else if (xmlStrEqual(n->name, (const xmlChar *)"tab")) {
			fputc('\t', outfile);
		} else if (xmlStrEqual(n->name, (const xmlChar *)"br") || xmlStrEqual(n->name, (const xmlChar *)"cr")) {
			fputc('\n', outfile);
		} else if (xmlStrEqual(n->name, (const xmlChar *)"p")) {
			walk(n->children, outfile);
			if (n->parent == NULL || !xmlStrEqual(n->parent->name, (const xmlChar *)"tc"))
				fputc('\n', outfile);
		} else if (xmlStrEqual(n->name, (const xmlChar *)"tr")) {
			walk(n->children, outfile);
			fputc('\n', outfile);
		} else if (xmlStrEqual(n->name, (const xmlChar *)"tc")) {
			walk(n->children, outfile);
			for (next = n->next; next && next->type != XML_ELEMENT_NODE; next = next->next)
				;
			if (next)
				fputc('\t', outfile);   /* separate table cells */
		} else {
			walk(n->children, outfile);
		}
	}
}

void
parsexml(const char *data, size_t len, FILE *outfile)
{
	xmlDocPtr document;
	xmlNode *root, *node_body;

	document = xmlReadMemory(data, (int)len, "document.xml", NULL, 0);
	if (document == NULL) {
		die("Unable to read xml file");
	}

	root = xmlDocGetRootElement(document);

	if (root == NULL) {
		die("No root in xml file");
	}

	if (!xmlStrEqual(root->name, (const xmlChar *) "document")) {
		die("wrong xml format");
	}
	for (node_body = root->children; node_body; node_body = node_body->next) {
		if (xmlStrEqual(node_body->name, (const xmlChar *) "body")) {
			walk(node_body->children, outfile);
		}
	}

	xmlFreeDoc(document);
	xmlCleanupParser();
	return;
}

static void
usage(void)
{
	die("usage: doctxt infile [-o outfile]");
}

int
main(int argc, char *argv[])
{
	FILE *outfile = NULL;
	const char *outfilename = "out.txt";
	const char *infilename = NULL;
	size_t len;
	char *data;
	int i;

	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-v")) {
			puts("doctxt-"VERSION);
			return 0;
		} else if (!strcmp(argv[i], "-o")) {
			if (i + 1 >= argc) {
				usage();
			}
			outfilename = argv[++i];
		} else if (argv[i][0] == '-') {
			usage();
		} else {
			if (infilename != NULL) {
				usage();
			}
			infilename = argv[i];
		}
	}

	if (infilename == NULL) {
		usage();
	}

	data = readzip(infilename, "word/document.xml", &len);

	if ((outfile = fopen(outfilename, "w")) == NULL) {
		die("Unable to open %s:", outfilename);
	}

	parsexml(data, len, outfile);
	fclose(outfile);
	free(data);

	return 0;
}
