#include "resource_lookup.h"

#include "logging.h"

#include <stdio.h>
#include <string.h>

// ════════════════════════════════════════════════════════════
// FONT REGISTRY
// ════════════════════════════════════════════════════════════

typedef struct
{
  const char *name;
  const char *path;
} resource_entry_t;

// Explicit size declaration to help static analyzer understand sentinel pattern
static const resource_entry_t font_registry[5] = {
  { "liberation_sans_bold", "resources/fonts/LiberationSans-Bold.ttf" },
  { "b612_mono_bold", "resources/fonts/B612Mono-Bold.ttf" },
  { "share_tech_mono", "resources/fonts/ShareTechMono-Regular.ttf" },
  { "orbitron_bold", "resources/fonts/Orbitron-Bold.ttf" },
  { NULL, NULL } // Sentinel
};

const char *
get_font_path(const char *font_name)
{
  if (!font_name || font_name[0] == '\0')
    {
      return NULL;
    }

  // NOLINTNEXTLINE(clang-analyzer-security.ArrayBound)
  // False positive: Sentinel pattern is valid, loop terminates at NULL entry
  for (int i = 0; font_registry[i].name != NULL; i++)
    {
      if (strcmp(font_registry[i].name, font_name) == 0)
        {
          return font_registry[i].path;
        }
    }

  LOG_WARN("Font not found: %s", font_name);
  return NULL;
}

void
list_available_fonts(void)
{
  printf("Available fonts:\\n");
  // NOLINTNEXTLINE(clang-analyzer-security.ArrayBound)
  // False positive: Sentinel pattern is valid, loop terminates at NULL entry
  for (int i = 0; font_registry[i].name != NULL; i++)
    {
      printf("  - %s → %s\\n", font_registry[i].name, font_registry[i].path);
    }
}
