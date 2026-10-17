# KMLDonkey — Qt6 / KDE Frameworks 6 port

A community port of the classic **KMLDonkey** frontend to **Qt6 and KDE
Frameworks 6**, intended for use with Plasma 6 on Linux.
This is an independent fork, not an official KDE release.

KMLDonkey connects to a local or remote MLDonkey core to manage searches,
downloads, servers, shared files, friends, transfer statistics, and its console.

The desktop interface uses **Qt Widgets and KDE Frameworks 6**, with top
navigation tabs, theme icons and refreshed dialogs. Appearance follows the
active Qt style, icon theme and color palette, including dark themes.
Navigation tabs are selected on the first start of the refreshed version;
the View menu also offers a sidebar and subsequent starts remember the choice.

## What changed in this fork

- Refreshed the desktop interface with top navigation tabs, theme icons, spaced
  search forms, frameless file lists, and a simplified status bar. Use
  **View → Sidebar Navigation** for a sidebar; the choice is saved.
- Migrated the desktop application and protocol library from Qt4 / KDE4 to Qt6 / KDE Frameworks 6.
- Removed the obsolete Plasma 4 applet and Nokia N9/MeeGo interface.
- Replaced obsolete dialogs, application startup, tray integration, and desktop APIs.
- Replaced KHTML with a Qt HTML viewer with history and an external browser option.
- Fixed download list updates, search field decoding, and ED2K link handling.
- Added one destination for completed downloads, with separate paths under advanced settings and permission checks performed by the core user.
- Fixed main-window ownership during shutdown and Return-key handling in search combo boxes.
- Save window geometry, the selected page, panel sizes, and view headers before shutdown clears the interface.
- Refreshed configuration dialogs with KDE standard buttons, category icons,
  per-category option search, descriptions and checkbox editors for booleans.
- Initialize KDE style and icon integration, use KDE password entry and text
  prompts, and follow the desktop palette in transfer graphs.
- Corrected folder saving and error replies; OK closes after a successful save, or immediately when nothing has changed.
- Suppressed disconnection error notifications during normal application shutdown.
- Added automated protocol and interface checks and a script that creates a `.deb` with runtime dependencies.

The source baseline is the official KDE SVN tree at revision **1531665**,
last changed on **29 December 2018**. The application declares version **2.1.5**;
this fork's Qt6 package is version **2.1.5+qt6.12**.

## Download folders

Open **Settings → Configure MLDonkey… → Paths** to choose one folder for
all completed downloads. **Advanced folder settings** exposes separate
folders for individual files, multi-file downloads, temporary data, and other
path options. Existing separate folders are preserved until you choose a
common destination. Local cores have a folder chooser; for remote cores,
enter a path on the server.

The core checks that each folder exists and can be written by its account
before applying the paths. This requires the companion
[MLDonkey OCaml 5 port](https://github.com/bettisfr/mldonkey/tree/ocaml5) package
**3.2.1+ocaml5.5** or newer. Existing shared folders and their strategies are
preserved, including their priorities and network restrictions. Existing files
are not moved; leave the temporary folder unchanged while downloads are running.
For a local core running as a system service, the destination must also be
accessible to the service account, including traversal of its parent folders.

## Status and compatibility

**Experimental, with initial checks on Ubuntu 26.04 amd64.** The desktop
application builds against Qt 6.10.2 and KDE Frameworks 6.24.0. Package generation
and startup of the packaged executable outside the source tree have also been
checked. Automated checks
cover a simulated core connection, fragmented protocol messages, searches,
active and completed downloads, console commands, ED2K links, web navigation,
and the main dialogs. Initial manual use with the companion MLDonkey 3.2.1
OCaml 5 port has exercised searches, live downloads, completed files, and folder
configuration. Broader testing and regression checks of the latest fixes are
still needed.

The package uses the libraries available on the build system; a `.deb` built on
Ubuntu 26.04 is not a generic package for all Debian and Ubuntu releases.

Known limitations:

- The HTML viewer does not execute JavaScript. Complex sites can be opened in
  the system browser.
- The internal multimedia preview was already disabled in the upstream snapshot.
  The existing Preview/Open actions request the backend's previewer; they do
  not provide an embedded player or launch a player in the frontend session.
- Saved automatic-renaming rules are not restored on startup yet.
- The frontend negotiates GUI protocol 33 with newer cores; later protocol
  extensions are not implemented.
- Application translations are not bundled in this source snapshot; the interface
  uses its English source strings.

## Build and install

Clone the fork:

```sh
git clone https://github.com/bettisfr/kmldonkey.git
cd kmldonkey
```

Install the dependencies listed in [BUILDING.md](BUILDING.md), then run:

```sh
./scripts/build-deb.sh
sudo apt install ./build-qt6/kmldonkey_2.1.5+qt6.12_amd64.deb
```

The filename above is for amd64; use the generated filename on other architectures.
APT resolves the declared runtime dependencies. The package installs in the
standard system paths under `/usr`, including `/usr/bin/kmldonkey` and the KDE
menu entry. It is installed as `kmldonkey` and replaces an existing package of
that name. MLDonkey is recommended; the core can also run on another machine.

Start it from the application menu or run:

```sh
kmldonkey
```

Configure the core address, GUI port, username, and password through the
connection settings. The default GUI port is `4001`.

To uninstall:

```sh
sudo apt remove kmldonkey
```

See [BUILDING.md](BUILDING.md) for packaging details and test instructions.

## Contributing

When reporting an issue, include the distribution and version, architecture,
Qt / KDE Frameworks versions, MLDonkey core version, steps to reproduce the
problem, and relevant build or terminal output.

## Credits and license

**Francesco Betti Sorbelli** — Qt6 / KDE Frameworks 6 port:
[bettisfr on GitHub](https://github.com/bettisfr).

This fork builds on the original KMLDonkey authors and KDE contributors.
Existing copyright and license notices are preserved in the sources;
see [AUTHORS](AUTHORS) and [COPYING](COPYING).
