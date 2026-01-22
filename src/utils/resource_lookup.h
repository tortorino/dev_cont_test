#ifndef RESOURCE_LOOKUP_H
#define RESOURCE_LOOKUP_H

// Resource lookup utilities
// Maps simple names to full resource paths for fonts

#include <stdbool.h>

// Get full path for a font name
// Returns path string on success, NULL if font not found
const char *get_font_path(const char *font_name);

// List all available fonts (for validation/debugging)
void list_available_fonts(void);

#endif // RESOURCE_LOOKUP_H
