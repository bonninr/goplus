/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOHauptwerkAttributeDict.h"

#include <map>

// The dictionary itself, generated from ODFEdit's HwObjectsAttributesDict.txt
extern const char *const GO_HAUPTWERK_ATTRIBUTE_DICT;

namespace {

using GOAttributesByCode = std::map<wxString, wxString>;
using GODictionary = std::map<wxString, GOAttributesByCode>;

GODictionary &getDictionary() {
  static GODictionary dictionary;

  return dictionary;
}

bool isLoaded = false;

} // namespace

/**
 * The dictionary is written one entry per line, in a JSON-like shape with
 * trailing commas:
 *
 *     "Combination":
 *      {
 *      "a":"CombinationID",
 *      ...
 *
 * which is read line by line rather than parsed as JSON, because the
 * trailing commas make it invalid JSON and there is nothing else in it.
 */
void GOHauptwerkAttributeDict::EnsureLoaded() {
  if (!isLoaded) {
    GODictionary &dictionary = getDictionary();
    const char *p = GO_HAUPTWERK_ATTRIBUTE_DICT;
    GOAttributesByCode *pAttributes = nullptr;

    while (*p) {
      const char *lineStart = p;

      while (*p && *p != '\n')
        p++;
      wxString line = wxString::FromUTF8(lineStart, (size_t)(p - lineStart));
      if (*p)
        p++;
      line.Trim(true).Trim(false);
      if (!line.StartsWith(wxT('"')))
        continue;
      // The code before the first colon; a line of "type": opens a type and
      // a line of "code":"name", one entry.
      const size_t colonI = line.find(wxT(':'));
      if (colonI == wxString::npos)
        continue;
      wxString key = line.Mid(1, colonI - 2);
      wxString value;

      if (colonI + 1 < line.Length()) {
        const size_t valueStart = line.find(wxT('"'), colonI + 1);

        if (valueStart != wxString::npos) {
          const size_t valueEnd = line.find(wxT('"'), valueStart + 1);

          if (valueEnd != wxString::npos)
            value = line.Mid(valueStart + 1, valueEnd - valueStart - 1);
        }
      }
      /* A type opens with "Name": and an entry is "code":"value", but the
       * dictionary also states "IDattr":"" for the types that have no id of
       * their own, which is an empty value and must not be read as a type. */
      if (value.IsEmpty() && key != wxT("IDattr"))
        pAttributes = &dictionary[key];
      else if (pAttributes)
        (*pAttributes)[key] = value;
    }
    isLoaded = true;
  }
}

wxString GOHauptwerkAttributeDict::Lookup(
  const wxString &objectType, const wxString &code) {
  EnsureLoaded();

  wxString result;
  const GODictionary &dictionary = getDictionary();
  const auto typeIt = dictionary.find(objectType);

  if (typeIt != dictionary.end()) {
    const auto attributeIt = typeIt->second.find(code);

    if (attributeIt != typeIt->second.end())
      result = attributeIt->second;
  }
  return result;
}
