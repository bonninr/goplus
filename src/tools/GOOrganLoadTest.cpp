/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

/*
 * Loads an organ and reports what came out, without starting audio or MIDI.
 *
 * The application asks the user to configure a sound device when it cannot
 * open one, which on a machine with no sound card means a dialog appears and
 * the organ is never reached - so the GUI is not the place to check whether a
 * definition loads. This builds the model directly, which is also what makes
 * the check fast enough to run on every change.
 */

#include <iostream>

#include <wx/app.h>
#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/image.h>
#include <wx/log.h>

#include "config/GOConfig.h"
#include "loader/GOProgressMonitor.h"
#include "model/GOManual.h"
#include "model/GORank.h"
#include "model/GOStop.h"
#include "model/GOSwitch.h"
#include "model/GOTremulant.h"

#include "gui/panels/GOGUIPanel.h"
#include "model/GOWindchest.h"

#include "GOOrgan.h"
#include "GOOrganController.h"

class GOSilentProgress : public GOProgressMonitor {
public:
  void Setup(long, const wxString &, const wxString &) override {}
  void Reset(long, const wxString &) override {}
  bool Update(unsigned, const wxString &) override { return true; }
};

class GOOrganLoadTestApp : public wxApp {
public:
  bool OnInit() override {
    wxLog::SetActiveTarget(new wxLogStream(&std::cerr));
    wxImage::AddHandler(new wxJPEGHandler);
    wxImage::AddHandler(new wxPNGHandler);
    wxImage::AddHandler(new wxGIFHandler);
    wxImage::AddHandler(new wxBMPHandler);
    return true;
  }

  int OnRun() override {
    int result = 0;
    wxString organPath;
    wxString workDir;
    bool isWindModel = false;
    bool isNoVoicing = false;
    bool isNoSwitches = false;
    bool isNoTremulantModel = false;
    bool isNoConsole = false;
    bool isLoadSamples = false;
    bool isStream = false;
    bool isBoundedBuild = false;
    bool isKeepCache = false;
    unsigned headKb = 256;

    for (int i = 1; i < argc; i++) {
      const wxString arg = argv[i];

      if (arg == wxT("--work-dir") && i + 1 < argc)
        workDir = argv[++i];
      else if (arg == wxT("--wind-model"))
        isWindModel = true;
      else if (arg == wxT("--no-voicing"))
        isNoVoicing = true;
      else if (arg == wxT("--no-switches"))
        isNoSwitches = true;
      else if (arg == wxT("--no-tremulant-model"))
        isNoTremulantModel = true;
      else if (arg == wxT("--no-console"))
        isNoConsole = true;
      else if (arg == wxT("--load-samples"))
        isLoadSamples = true;
      else if (arg == wxT("--stream"))
        isStream = true;
      else if (arg == wxT("--bounded-build"))
        isBoundedBuild = true;
      else if (arg == wxT("--keep-cache"))
        isKeepCache = true;
      else if (arg == wxT("--head-kb") && i + 1 < argc)
        headKb = wxAtoi(argv[++i]);
      else if (!arg.StartsWith(wxT("-")))
        organPath = arg;
    }

    if (organPath.IsEmpty()) {
      std::cerr << "Usage: GOOrganLoadTest [--work-dir DIR] <organ file>\n";
      result = 2;
    } else {
      if (workDir.IsEmpty())
        workDir = wxFileName::GetTempDir() + wxFileName::GetPathSeparator()
          + wxT("goloadtest");
      wxFileName::Mkdir(workDir, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);

      /* A cold cache unless asked otherwise, so that a run measures building
       * it as well - which is the case a small machine actually fails on. */
      if (!isKeepCache) {
        wxArrayString stale;
        wxDir::GetAllFiles(workDir, &stale);
        for (size_t i = 0; i < stale.GetCount(); i++)
          wxRemoveFile(stale[i]);
      }

      const std::string confPath
        = std::string(workDir.mb_str()) + "/GrandOrgue.conf";
      GOConfig config("loadtest", confPath);

      config.Load();
      config.OrganCachePath(workDir);
      config.OrganSettingsPath(workDir);
      /* Reading the samples takes minutes, so it only happens when asked:
       * the definition is either understood or it is not long before then.
       * With samples the run also exercises the cache and its streaming
       * modes, which is the other half of what a load can get wrong. */
      config.ManageCache(isLoadSamples);
      config.CompressCache(false);
      config.StreamFromCache(isStream);
      config.StreamHeadKB(headKb);
      config.BoundedCacheBuild(isBoundedBuild);
      // Set here rather than in the config file so a run states its own
      // conditions and leaves nothing behind for the next one.
      config.HauptwerkWindModel(isWindModel);
      config.HauptwerkVoicing(!isNoVoicing);
      config.HauptwerkSwitches(!isNoSwitches);
      config.HauptwerkTremulantModel(!isNoTremulantModel);
      config.HauptwerkConsole(!isNoConsole);

      // True, not false: the panels are built during Load and reach for the
      // image cache, which only exists when the controller is told the
      // application is up. With false it is null and the load segfaults.
      GOOrganController controller(config, true);
      GOOrgan organ(organPath);
      GOSilentProgress monitor;
      /* Without samples the load stops before the objects, which is all the
       * definition check needs; with them the whole organ is built and the
       * cache modes run their course. */
      const wxString errMsg
        = controller.Load(organ, wxEmptyString, !isLoadSamples, monitor);

      if (!errMsg.IsEmpty()) {
        std::cout << "LOAD FAILED: " << errMsg.ToUTF8().data() << "\n";
        result = 1;
      } else {
        std::cout << "LOAD OK\n";
        std::cout << "  organ    : "
                  << controller.GetOrganName().ToUTF8().data() << "\n";
        std::cout << "  manuals  : " << controller.GetManualAndPedalCount()
                  << " (first " << controller.GetFirstManualIndex() << ")\n";
        std::cout << "  ranks    : " << controller.GetODFRankCount() << "\n";
        std::cout << "  windchsts: " << controller.GetWindchestCount() << "\n";
        std::cout << "  enclosurs: " << controller.GetEnclosureCount() << "\n";
        std::cout << "  tremulnts: " << controller.GetTremulantCount() << "\n";
        std::cout << "  switches : " << controller.GetSwitchCount() << "\n";

        unsigned nStops = 0;
        unsigned nCouplers = 0;

        for (unsigned manualI = controller.GetFirstManualIndex();
             manualI <= controller.GetManualAndPedalCount();
             manualI++) {
          GOManual *pManual = controller.GetManual(manualI);

          if (pManual) {
            nStops += pManual->GetStopCount();
            nCouplers += pManual->GetCouplerCount();
            std::cout << "  manual " << manualI << " : "
                      << pManual->GetName().ToUTF8().data() << ", "
                      << pManual->GetStopCount() << " stops, "
                      << pManual->GetCouplerCount() << " couplers, keys "
                      << pManual->GetFirstAccessibleKeyMIDINoteNumber() << "+"
                      << pManual->GetNumberOfAccessibleKeys() << "\n";
          }
        }
        std::cout << "  stops    : " << nStops << "\n";

        /* The numbers are what --render-stops takes, so they are printed
         * with the names to choose a registration from. */
        unsigned stopN = 0;

        for (unsigned manualI = controller.GetFirstManualIndex();
             manualI <= controller.GetManualAndPedalCount();
             manualI++) {
          GOManual *pManual = controller.GetManual(manualI);

          if (pManual)
            for (unsigned nStops = pManual->GetStopCount(), stopI = 0;
                 stopI < nStops;
                 stopI++) {
              GOStop *pStop = pManual->GetStop(stopI);

              if (pStop)
                std::cout << "  stop " << ++stopN << " : "
                          << pStop->GetName().ToUTF8().data() << " ("
                          << pManual->GetName().ToUTF8().data() << ")\n";
            }
        }
        std::cout << "  couplers : " << nCouplers << "\n";
        std::cout << "  voicing  : " << (isNoVoicing ? "off" : "on") << "\n";
        std::cout << "  windmodel: " << (isWindModel ? "on" : "off") << "\n";

        unsigned nWindLimited = 0;
        unsigned nTremulantChests = 0;

        for (unsigned n = controller.GetWindchestCount(), chestI = 0;
             chestI < n;
             chestI++) {
          GOWindchest *pChest = controller.GetWindchest(chestI);

          if (pChest) {
            if (pChest->HasWindModel())
              nWindLimited++;
            if (pChest->GetTremulantCount() > 0)
              nTremulantChests++;
          }
        }
        std::cout << "  wind-limited chests: " << nWindLimited << "\n";
        std::cout << "  tremulant chests: " << nTremulantChests << "\n";

        for (unsigned n = controller.GetTremulantCount(), tremI = 0; tremI < n;
             tremI++) {
          const GOTremulant *pTremulant = controller.GetTremulant(tremI);

          if (pTremulant)
            std::cout << "  tremulant " << tremI << " : amp depth "
                      << pTremulant->GetAmpModDepth() << "%, pitch depth "
                      << pTremulant->GetPitchModDepth() << " cents\n";
        }

        unsigned nDrawnSwitches = 0;
        unsigned nDerivedSwitches = 0;

        for (unsigned n = controller.GetSwitchCount(), switchI = 0; switchI < n;
             switchI++) {
          const GOSwitch *pSwitch = controller.GetSwitch(switchI);

          if (pSwitch) {
            if (pSwitch->IsDisplayed())
              nDrawnSwitches++;
            if (pSwitch->IsReadOnly())
              nDerivedSwitches++;
          }
        }
        std::cout << "  switches drawn: " << nDrawnSwitches
                  << ", derived: " << nDerivedSwitches << "\n";

        for (unsigned n = controller.GetPanelCount(), panelI = 0; panelI < n;
             panelI++)
          std::cout << "  panel " << panelI << " : "
                    << controller.GetPanel(panelI)->GetName().ToUTF8().data()
                    << "\n";

        GOMemoryPool &pool = controller.GetMemoryPool();

        std::cout << "  pool alloc MB : "
                  << (pool.GetAllocSize() / (1024.0 * 1024.0)) << "\n";
        std::cout << "  mapped cache MB: "
                  << (pool.GetMappedSize() / (1024.0 * 1024.0)) << "\n";
        std::cout << "  pool usage MB : "
                  << (pool.GetPoolUsage() / (1024.0 * 1024.0)) << "\n";
        std::cout << "  stream        : "
                  << (pool.IsStreamFromCache() ? "yes" : "no") << "\n";
      }
      controller.Clear();
    }
    return result;
  }
};

DECLARE_APP(GOOrganLoadTestApp)
IMPLEMENT_APP_CONSOLE(GOOrganLoadTestApp)
