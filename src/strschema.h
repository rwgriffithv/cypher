/**
 * @file strschema.h
 * @author Rob Griffith
 */

#ifndef STRSCHEMA_H
#define STRSCHEMA_H

#include <stddef.h>

/**
 * @enum strschema_mod
 * @brief modifiers applied to string schema nodes
 * @typedef strschema_mod_t
 */
typedef enum strschema_mod
{
    SSMOD_NODE_SINGLE, /** typical single instance of node required */
    SSMOD_NODE_PLUS,   /** one or more instances of node required */
    SSMOD_NODE_ANY,    /** any number of instances of node allowed */
    SSMOD_SUBNODE_OR,  /** one or more of different subnodes are required */
    SSMOD_SUBNODE_XOR  /** exclusively one of different subnodes is required */
} strschema_mod_t;

/**
 * @struct strschema
 * @brief node in string schema context tree for validating and parsing
 * @typedef strschema_t
 *
 * Used to define simple hierarchical regex-like string schemas.
 * e.g. URI definition (RFC 3986)
 * URI = scheme ":" hier-part [ "?" query ] [ "#" fragment ]
 * scheme = ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )
 * ...
 *
 * String schema is defined by root node.
 * Modifiers allow for regex-like statements.
 * Nodes with modifiers NODE_PLUS or NODE_ANY will contain the full string,
 * but subnodes will contain their respective last-parsed substring. Use
 * @ref strschema_parse_one to iterate over a parsed NODE_PLUS or NODE_ANY.
 */
typedef struct strschema
{
    /** modifier applying to node */
    strschema_mod_t mod;
    /** sub nodes, NULL if leaf node */
    strschema_t *children;
    /** number of sub nodes, 0 if leaf node */
    size_t nchildren;
    /** character membership function, NULL if not leaf node, return 0 if not member */
    int (*ismember)(char c);
    /** parsed string for node, NULL if not found */
    const char *str;
    /** parsed string length, no null byte */
    size_t len;
} strschema_t;

/**
 * @brief parse/validate function that recursively goes over tree
 *
 * @param str the string to parse
 * @param[inout] schema the string schema context to use and populate
 * @return populated schema context, NULL if incorrectly formatted string
 */
const strschema_t *srtschema_parse(const char *str, strschema_t *schema);

/**
 * @todo
 * @brief parse/validate function that recursively goes over tree
 *
 * @param str the string to parse
 * @param[inout] schema the string schema context to use and populate
 * @param len the number of non-null characters in str
 * @return populated schema context, NULL if incorrectly formatted string
 */
const strschema_t *srtschema_parsen(const char *str, size_t len, strschema_t *schema);

/**
 * @todo
 * @brief parse exactly one instance of previously parsed schema node
 *
 * This is primarily for iterating over instances of a node with modifier
 * NODE_PLUS or NODE_ANY that was already parsed, extracting individual values.
 * Subnodes are populated according to their own modifiers as usual.
 *
 * Input string and length should be initial full parsed string for NODE_PLUS
 * or NODE_ANY node, and will be incremented to position in string after
 * section parsed as node instance. Input string will be set to NULL upon
 * completion, NULL only returned upon error.
 * e.g.
 * const char* str = node->str;
 * size_t len = node->len;
 * while (str) {
 *     if (!srtschema_parse_one(str, len, node))
 *     {
 *         <handle error>
 *     }
 *     <do work with set node->str, node->len and anything from subnodes>
 * }
 *
 * @param[inout] str the string to parse
 * @param[inout] len the number of non-null characters in str
 * @param[inout] schema the string schema context to use and populate
 * @return populated schema context, NULL if incorrectly formatted string
 */
const strschema_t *srtschema_parse_one(char **str, size_t *len, strschema_t *schema);

#endif