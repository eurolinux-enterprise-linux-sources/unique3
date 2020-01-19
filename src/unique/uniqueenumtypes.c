
/* Generated data (by glib-mkenums) */

#include "uniqueenumtypes.h"

/* enumerations from "../unique/uniqueapp.h" */
#include "../unique/uniqueapp.h"

GType
unique_command_get_type(void) {
  static volatile gsize g_enum_type_id__volatile = 0;

  if (g_once_init_enter (&g_enum_type_id__volatile))
    {
      static const GEnumValue values[] = {
        { UNIQUE_INVALID, "UNIQUE_INVALID", "invalid" },
        { UNIQUE_ACTIVATE, "UNIQUE_ACTIVATE", "activate" },
        { UNIQUE_NEW, "UNIQUE_NEW", "new" },
        { UNIQUE_OPEN, "UNIQUE_OPEN", "open" },
        { UNIQUE_CLOSE, "UNIQUE_CLOSE", "close" },
        { 0, NULL, NULL }
      };
      GType g_enum_type_id;

      g_enum_type_id =
        g_enum_register_static (g_intern_static_string ("UniqueCommand"), values);

      g_once_init_leave (&g_enum_type_id__volatile, g_enum_type_id);
    }

  return g_enum_type_id__volatile;
}
GType
unique_response_get_type(void) {
  static volatile gsize g_enum_type_id__volatile = 0;

  if (g_once_init_enter (&g_enum_type_id__volatile))
    {
      static const GEnumValue values[] = {
        { UNIQUE_RESPONSE_INVALID, "UNIQUE_RESPONSE_INVALID", "invalid" },
        { UNIQUE_RESPONSE_OK, "UNIQUE_RESPONSE_OK", "ok" },
        { UNIQUE_RESPONSE_CANCEL, "UNIQUE_RESPONSE_CANCEL", "cancel" },
        { UNIQUE_RESPONSE_FAIL, "UNIQUE_RESPONSE_FAIL", "fail" },
        { UNIQUE_RESPONSE_PASSTHROUGH, "UNIQUE_RESPONSE_PASSTHROUGH", "passthrough" },
        { 0, NULL, NULL }
      };
      GType g_enum_type_id;

      g_enum_type_id =
        g_enum_register_static (g_intern_static_string ("UniqueResponse"), values);

      g_once_init_leave (&g_enum_type_id__volatile, g_enum_type_id);
    }

  return g_enum_type_id__volatile;
}

/* Generated data ends here */

