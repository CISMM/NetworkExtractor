#ifndef __XML_HELPER_H_
#define __XML_HELPER_H_

#include <libxml/tree.h>

/**
 * Finds the first child node of an XML tree node with a given name.
 * Returns NULL if a child with that name does not exist.
 *
 * @param root Root node of XML tree.
 * @param name Name of desired child.
 */
xmlNode* FindChildNodeWithName(xmlNode* root, const char* name);


#endif // __XML_HELPER_H_