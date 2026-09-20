/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOHAUPTWERKATTRIBUTEDICT_H
#define GOHAUPTWERKATTRIBUTEDICT_H

#include <wx/string.h>

/**
 * The attribute dictionary of Hauptwerk's compressed definition format.
 *
 * A compressed definition writes most of its objects as <o> with one- and
 * two-letter element names instead of the full attribute names, which makes
 * the file several times smaller. The letters carry no rule: the same letter
 * means different attributes in different object types, so only the
 * dictionary that shipped with the format - the one ODFEdit decodes with -
 * can tell what they are.
 */
class GOHauptwerkAttributeDict {
public:
  /**
   * @param objectType the ObjectType of the enclosing ObjectList
   * @param code the element name as the compressed file spells it
   * @return the full attribute name, or an empty string when the dictionary
   *   does not know the code
   */
  static wxString Lookup(const wxString &objectType, const wxString &code);

private:
  static void EnsureLoaded();
};

#endif /* GOHAUPTWERKATTRIBUTEDICT_H */
