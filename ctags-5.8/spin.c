/*
 *   $Id:$
 *
 *   Copyright (c) 2012, Steve Denson
 *
 *   Built on work for basic.c created as
 *   Copyright (c) 2000-2006, Darren Hiebert, Elias Pschernig
 *
 *   This source code is released for free distribution under the terms of the
 *   GNU General Public License.
 *
 *   This module contains functions for generating tags for Spin.
 *   For now, this is kept
 *   quite simple - but feel free to ask for more things added any time -
 *   patches are of course most welcome.
 */

/*
 *   INCLUDE FILES
 */
#include "general.h" /* must always come first */

#include <string.h>

#include "options.h"
#include "parse.h"
#include "read.h"
#include "routines.h"
#include "vstring.h"

/*
 *   DATA DEFINITIONS
 */
typedef enum {
    K_NONE,
    K_CONST,
    K_FUNCTION,
    K_OBJECT,
    K_TYPE,
    K_VAR,
    //K_DAT,
    K_ENUM
} SpinKind;

typedef struct {
    char const *token;
    SpinKind kind;
    int skip;
} KeyWord;

static kindOption SpinKinds[] = {
    {FALSE,'n', "none", "none"}, // place-holder only
    {TRUE, 'c', "constant", "constants"},
    {TRUE, 'f', "function", "functions"},
    {TRUE, 'o', "obj", "objects"},
    {TRUE, 't', "type", "types"},
    {TRUE, 'v', "var", "variables"},
    //{TRUE, 'd', "dat", "dat"},
    {TRUE, 'g', "enum", "enumerations"}
};

#define BYTE "byte"
#define LONG "long"
#define WORD "word"

static KeyWord spin_keywords[] = {
    {"con", K_CONST, 0},
    {"obj", K_OBJECT, 0},
    {"pub", K_FUNCTION, 0},
    {"pri", K_FUNCTION, 0},
    {"var", K_VAR, 0},
    //{"dat", K_DAT, 0},
    {NULL, 0, 0}
};

/*
 *   FUNCTION DEFINITIONS
 */

/* Match the name of a tag (function, variable, type, ...) starting at pos. */
static char const *extract_name (char const *pos, vString * name)
{
    while (isspace (*pos))
        pos++;
    vStringClear (name);
    for (; *pos && !isspace (*pos) && *pos != '(' && *pos != ','; pos++)
        vStringPut (name, *pos);
    vStringTerminate (name);
    return pos;
}

static int tokentype(char *tmp)
{
    int rc = K_NONE;
    int i;
    /* find out token type for parser states */
    /* printf ("token %s\n", tmp); */
    for (i = 0; spin_keywords[i].token != NULL; i++)
    {
        int len = strlen(tmp);
        if(strchr(tmp, ' ') != 0) {
            len = (int) (strchr(tmp, ' ')-tmp);
            tmp[len] = '\0';
        }
        if (strcasecmp(tmp, spin_keywords[i].token) == 0) {
            rc = spin_keywords[i].kind;
            /* printf ("keyword type %d\n", rc); */
            break;
        }
    }
    return rc;
}

/* Match a keyword starting at p (case insensitive). */
static int match_keyword (const char *p, KeyWord const *kw)
{
    vString *name;
    size_t i;
    int j;
    int rc = K_NONE;
    char *tmp = (char*)p;

    /* printf ("match_keyword\n"); */

    for (i = 0; i < strlen (kw->token); i++)
    {
        if (tolower (p[i]) != kw->token[i])
            return rc;
    }

    p += i;

    /* if p stands alone on the line, don't add it */
    if(strlen(p) == 0)
         return tokentype(tmp);

    name = vStringNew ();
    for (j = 0; j < 1 + kw->skip; j++)
    {
        p = extract_name (p, name);
    }    
    /* printf ("match_keyword name %d %s\n", name->length, name->buffer); */

    if(name->length > 0) {
        makeSimpleTag (name, SpinKinds, kw->kind);
    }
    vStringDelete (name);

    rc = tokentype(tmp);

    return rc;
}

static int  spintype(char const *p)
{
    int n,j;
    char *types[] = { BYTE, LONG, WORD, 0 };
    char *type;

    for(j = 0; types[j] != 0; j++) {
        type = types[j];
        for(n = 0; isspace(p[n]) != 0; n++) {
            if(tolower(p[n]) != type[n]) return 0;
        }
    }
    return 1;
}

static void match_constant (char const *p)
{
    char const *end = p + strlen (p) - 1;
    while((*end != '=') && (end >= p))
        end--;
    if(end > p)
    {
        vString *name = vStringNew ();
        vStringNCatS (name, p, end - p);
        makeSimpleTag (name, SpinKinds, K_CONST);
        vStringDelete (name);
    }
}

static void match_object (char const *p)
{
    char const *end = p + strlen (p) - 1;
    while((*end != ':') && (end >= p))
        end--;
    if(end > p)
    {
        vString *name = vStringNew ();
        vStringNCatS (name, p, end - p);
        makeSimpleTag (name, SpinKinds, K_OBJECT);
        vStringDelete (name);
    }
}

static void match_var (char const *p)
{
    char const *end = p + strlen (p) - 1;
    while((spintype(end) == 0) && (end >= p))
        end--;
    end++;
    if(end > p)
    {
        vString *name = vStringNew ();
        vStringNCatS (name, p, end - p);
        makeSimpleTag (name, SpinKinds, K_VAR);
        vStringDelete (name);
    }
}

static void findSpinTags (void)
{
    const char *name;
    SpinKind   state;
    const char *line;
    const char *extension = fileExtension (vStringValue (File.name));
    KeyWord *keywords;

    keywords = spin_keywords;
    state = K_CONST; // spin starts with CONST

    while ((line = (const char *) fileReadLine ()) != NULL)
    {
        const char *p = line;
        KeyWord const *kw;

        while (isspace (*p))
            p++;

        /* Empty line? */
        if (!*p)
            continue;

        //printf ("state %d\n", state);
        switch(state) {
            case K_CONST:
                match_constant(p);
            break;
            case K_OBJECT:
                match_object(p);
            break;
            case K_VAR:
                match_var(p);
            break;
            default:
            break;
        }

        /* In Spin, keywords always are at the start of the line. */
        for (kw = keywords; kw->token; kw++) {
            int type = (SpinKind) match_keyword (p, kw);
            if(type != K_NONE)
                state = type;
        }
    }
}

/* Create parser definition stucture */
extern parserDefinition* SpinParser (void)
{
    static const char *const extensions [] = { "spin", NULL };
    parserDefinition* const def = parserNew ("Spin");
    def->kinds = SpinKinds;
    def->kindCount = KIND_COUNT (SpinKinds);
    def->extensions = extensions;
    def->parser = findSpinTags;
    return def;
}

/* vi:set tabstop=4 shiftwidth=4 et: */
