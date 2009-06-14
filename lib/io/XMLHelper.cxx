#include "XMLHelper.h"
#include <cstring>


xmlNode* FindChildNodeWithName(xmlNode* root, const char* name) {
  xmlNode* current = root->children;
  while (current != NULL) {
    if (!strcmp((char *) current->name, name)) {
      return current;
    }
    current = current->next;
  }

  return NULL;
}