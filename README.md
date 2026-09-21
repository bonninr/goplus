[![Build preview](https://github.com/bonninr/goplus/actions/workflows/build-preview.yml/badge.svg)](https://github.com/bonninr/goplus/actions/workflows/build-preview.yml)
[![Preview release](https://img.shields.io/github/v/release/bonninr/goplus?include_prereleases&label=preview)](https://github.com/bonninr/goplus/releases)
[![License: GPL v2](https://img.shields.io/badge/License-GPL%20v2-blue.svg)](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html)

# GoPlus

GoPlus is a fork of [GrandOrgue](https://github.com/GrandOrgue/grandorgue), the
sample based pipe organ simulator. It plays the same organ definitions, caches
and settings as GrandOrgue and adds two things on top:

- **Hauptwerk sample sets.** An `.Organ_Hauptwerk_xml` definition loads through
  the ordinary organ dialog, in either the plain or the compressed spelling
  (the letter-coded tables are decoded with ODFEdit's attribute dictionary).
  The converter is part of the loader, so there is no intermediate file: the
  sample set is read in place and converted in memory. The console is drawn
  from the sample set's own artwork where it provides one, and a generic
  console is drawn otherwise. Manuals, couplers, tremulants, enclosures, the
  switch and combination systems, crossfades, per-pipe voicing and the pitch
  each sample was recorded at are all carried across. A pipe keeps all its
  recorded attacks - including the ones made with the tremulant running, which
  GrandOrgue plays while the tremulant is drawn - and the velocity shelves the
  voicer set. Processing that costs time on older machines - the wind model
  and per-pipe voicing - can be switched off.
- **Sample streaming.** With a fast SSD or NVMe disk, sample data is paged from
  the cache on demand instead of being loaded into memory, and the cache itself
  can be built without ever holding the whole organ in RAM. Sample sets larger
  than the machine's memory become playable this way. The options are in
  *Settings -> Options -> Cache*, and the same options are available on the
  command line (`--stream`, `--head-kb`, `--bounded-build`) for scripted or
  comparative runs.

Everything else - the GrandOrgue ODF format, the combination system, MIDI and
audio configuration, the recorder, the sample cache format - is the upstream
GrandOrgue code. The built program is still called `GrandOrgue`, so existing
shortcuts, file associations and configuration directories keep working.

## Rendering without a console

A run can play a MIDI file into the loaded organ and record the result, which
is how a sample set is tested or a take is produced without anyone at the
keyboard:

    GrandOrgue --play-midi piece.mid --record-audio take.wav "organ.Organ_Hauptwerk_xml"

The player uses its configured channel mapping rather than asking, recording
stops a tail after the last MIDI event (or at `--render-seconds` if the file
never ends), and the application exits when it is done. Combine it with the
streaming options above to record an organ larger than the machine's memory.

## Downloads

Preview builds for Windows, Linux (x86_64 and Raspberry Pi) and macOS are
published as a rolling prerelease:
[https://github.com/bonninr/goplus/releases](https://github.com/bonninr/goplus/releases).
The files are named `goplus-...` to distinguish them from official GrandOrgue
packages.

Streaming wants an SSD or NVMe disk; on a Raspberry Pi that means a USB SSD or
an NVMe hat, not an SD card.

## Reporting problems

This fork is not supported by the GrandOrgue project. Report anything wrong
here, not upstream: [https://github.com/bonninr/goplus/issues](https://github.com/bonninr/goplus/issues).

## Building from sources

See [BUILD.md](BUILD.md) for the upstream build instructions; they apply to this
fork unchanged.

## Credits and references

GoPlus is based on GrandOrgue by Milan Digital Audio and the GrandOrgue
contributors (GPL-2.0-or-later). The Hauptwerk definition support was written
for this fork; the interpretation of the format was cross-checked against
[ODFEdit](https://github.com/GrandOrgue/OdfEdit) and
[RustyPipes](https://github.com/dividebysandwich/rusty-pipes), which document
the format independently.

GrandOrgue itself is hosted at
[https://github.com/GrandOrgue/grandorgue](https://github.com/GrandOrgue/grandorgue);
its discussion forum and issue tracker remain the right place for upstream
behaviour. GrandOrgue was earlier hosted on SourceForge
[https://sourceforge.net/projects/ourorgan](https://sourceforge.net/projects/ourorgan),
and the pre-migration source is kept in the
[svn branch](https://github.com/GrandOrgue/grandorgue/tree/svn).

## External libraries

![ASIO](https://user-images.githubusercontent.com/19529533/139595107-954ae23d-fa37-4346-ada6-3f69f203dcd2.jpg)
