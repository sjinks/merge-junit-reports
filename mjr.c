#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

static long int nTests = 0;
static long int nAssertions = 0;
static long int nErrors = 0;
static long int nWarnings = 0;
static long int nFailures = 0;
static long int nSkipped = 0;
static bool hasWarnings = 0;
static bool hasAssertions = 0;

static const xmlChar* s_testsuites = BAD_CAST "testsuites";
static const xmlChar* s_testsuite = BAD_CAST "testsuite";

struct map {
    const char* name;
    long int* value;
    bool* flag;
};

static const struct map map[] = {
    { "tests",      &nTests,      NULL },
    { "assertions", &nAssertions, &hasAssertions },
    { "errors",     &nErrors,     NULL },
    { "warnings",   &nWarnings,   &hasWarnings },
    { "failures",   &nFailures,   NULL},
    { "skipped",    &nSkipped,    NULL }
};

static void updateCounters(const xmlNodePtr node)
{
    for (size_t i = 0; i < sizeof(map) / sizeof(map[0]); ++i) {
        char* v = (char*)xmlGetProp(node, BAD_CAST map[i].name);
        if (v) {
            if (map[i].flag) {
                *map[i].flag = true;
            }

            *map[i].value += atol(v);
            xmlFree(v);
        }
    }
}

static xmlDocPtr loadFile(const char* filename)
{
    xmlDocPtr doc = xmlReadFile(filename, NULL, XML_PARSE_NOBLANKS);
    if (doc == NULL) {
        fprintf(stderr, "Failed to parse %s\n", filename);
        return NULL;
    }

    xmlNodePtr root = xmlDocGetRootElement(doc);
    assert(root != NULL);

    if (xmlStrcmp(root->name, s_testsuites)) {
        fprintf(stderr, "Error processing %s: the root element must be <testsuites>, <%s> found\n", filename, (const char*)root->name);
        xmlFreeDoc(doc);
        return NULL;
    }

    for (xmlNodePtr node = root->children; node; node = node->next) {
        if (node->type == XML_ELEMENT_NODE) {
            if (xmlStrcmp(node->name, s_testsuite)) {
                fprintf(stderr, "Error processing %s: expected <testsuite> element, <%s> found\n", filename, (const char*)node->name);
                xmlFreeDoc(doc);
                return NULL;
            }
        }
    }

    return doc;
}

static void appendTestSuites(xmlDocPtr doc, xmlNodePtr newParent)
{
    assert(doc != NULL);
    assert(newParent != NULL);

    xmlNodePtr root = xmlDocGetRootElement(doc);
    assert(root != NULL);

    for (xmlNodePtr node = root->children; node; node = node->next) {
        if (node->type == XML_ELEMENT_NODE) {
            xmlNodePtr adopted = xmlDocCopyNode(node, newParent->doc, 1);
            assert(adopted != NULL);
            xmlAddChild(newParent, adopted);
            updateCounters(adopted);
        }
    }
}

bool dumpXMLAndFree(xmlDocPtr doc)
{
    int res = xmlDocFormatDump(stdout, doc, 1);
    xmlFreeDoc(doc);
    if (res == -1) {
        fprintf(stderr, "There was an error saving the XML\n");
        return false;
    }

    return true;
}

static void cleanup(void)
{
    xmlCleanupParser();
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        return 1;
    }

    LIBXML_TEST_VERSION
    atexit(&cleanup);

    xmlIndentTreeOutput = 1;

    if (argc == 2) {
        xmlDocPtr result = loadFile(argv[1]);
        if (result) {
            return dumpXMLAndFree(result) ? EXIT_SUCCESS : EXIT_FAILURE;
        }

        return EXIT_FAILURE;
    }

    xmlDocPtr result = xmlNewDoc(BAD_CAST "1.0");
    xmlNodePtr rootNode = xmlNewNode(NULL, s_testsuites);
    xmlDocSetRootElement(result, rootNode);
    xmlNodePtr testsuite = xmlNewChild(rootNode, NULL, s_testsuite, NULL);
    xmlNewProp(testsuite, BAD_CAST "name", BAD_CAST "Combined Report");

    for (int i = 1; i < argc; ++i) {
        xmlDocPtr doc = loadFile(argv[i]);
        if (doc) {
            appendTestSuites(doc, testsuite);
            xmlFreeDoc(doc);
        }
    }

    for (size_t i = 0; i < sizeof(map) / sizeof(map[0]); ++i) {
        if (map[i].flag == NULL || *map[i].flag == true) {
            char str[((CHAR_BIT * sizeof(long int) - 1) / 3 + 2)];
            sprintf(str, "%ld", *map[i].value);
            xmlNewProp(testsuite, BAD_CAST map[i].name, BAD_CAST str);
        }
    }

    return dumpXMLAndFree(result) ? EXIT_SUCCESS : EXIT_FAILURE;
}
