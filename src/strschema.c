/**
 * @file strschema.c
 * @author Rob Griffith
 */

#include "strschema.h"

#include <string.h>

const strschema_t *_parse_one_parent(char **str, size_t *len, strschema_t *schema)
{
    const strschema_t *rv, *temp;
    size_t i;
    switch (schema->mod)
    {
    case SSMOD_NODE_PLUS:
        // must parse at least one set of children (>= 1 nodes)
        rv = NULL;
        do
        {
            for (i = 0; i < schema->nchildren; i++)
            {
                if (!(temp = srtschema_parse_one(str, len, &(schema->children[i]))))
                {
                    // error parsing child node
                    break;
                }
            }
            rv = schema; // successfully parsed >= 1 nodes
        } while (temp);
        if (i < schema->nchildren && i != 0)
        {
            // error parsing child node following successful first child node
            rv = NULL;
        }
        break;
    case SSMOD_NODE_ANY:
        // acceptable to parse no sets of children (node not present)
        rv = schema;
        do
        {
            for (i = 0; i < schema->nchildren; i++)
            {
                if (!(temp = srtschema_parse_one(str, len, &(schema->children[i]))))
                {
                    break;
                }
            }
        } while (temp);
        if (i < schema->nchildren && i != 0)
        {
            // error parsing child node following successful first child node
            rv = NULL;
        }
        break;
    case SSMOD_SUBNODE_OR:
        // must parse at least one child node
        rv = NULL;
        for (i = 0; i < schema->nchildren; i++)
        {
            if (srtschema_parse_one(str, len, schema->children))
            {
                // succesfully parsed child node
                rv = schema;
            }
        }
        break;
    case SSMOD_SUBNODE_XOR:
        // must parse only one child node
        rv = NULL;
        for (i = 0; i < schema->nchildren; i++)
        {
            if (srtschema_parse_one(str, len, schema->children))
            {
                if (rv)
                {
                    // already parsed a child node
                    rv = NULL;
                    break;
                }
                rv = schema;
            }
        }
        break;
    case SSMOD_NODE_SINGLE:
    default:
        // must parse one instance of node
        rv = schema;
        for (i = 0; i < schema->nchildren; i++)
        {
            if (!srtschema_parse_one(str, len, schema->children))
            {
                // failed to parse child
                rv = NULL;
                break;
            }
        }
        break;
    }
    return rv;
}

const strschema_t *_parse_one_child(char **str, size_t *len, strschema_t *schema)
{
    /// TODO: handle changes to input str and len
    const strschema_t *rv;
    size_t i;
    switch (schema->mod)
    {
    case SSMOD_NODE_PLUS:
        /// TODO: impl
        break;
    case SSMOD_NODE_ANY:
        /// TODO: impl
        break;
    case SSMOD_SUBNODE_OR:
        // invalid (only valid with children), treat as single
    case SSMOD_SUBNODE_XOR:
        // invalid, treat as single
    case SSMOD_NODE_SINGLE:
    default:
        /// TODO: impl
        break;
    }
    /// TODO: handle different schema->mod fields NODE_SINGLE, NODE_PLUS, NODE_ANY
    for (i = 0; i < *len; i++)
    {
        if (!schema->ismember((*str)[i]))
        {
        }
    }
    schema->str = *str;
    schema->len = *len;
    return rv;
}

const strschema_t *srtschema_parse(const char *str, strschema_t *schema)
{
    return strschema_parsen(str, strlen(str), schema);
}

const strschema_t *srtschema_parsen(const char *str, size_t len, strschema_t *schema)
{
    /// TODO: invoke combos of strschema_parse_one and recursive stchema_parsen
}

const strschema_t *srtschema_parse_one(char **str, size_t *len, strschema_t *schema)
{
    const strschema_t *rv;
    if (!(str && *str && len && *len && schema))
    {
        // invalid inputs
        rv = NULL;
    }
    if (schema->children && schema->nchildren)
    {
        rv = _parse_one_parent(str, len, schema);
    }
    else if (schema->ismember)
    {
        rv = _parse_one_leaf(str, len, schema);
    }
    return rv;
}