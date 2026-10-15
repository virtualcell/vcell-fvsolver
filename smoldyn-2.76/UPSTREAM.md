# Vendored Smoldyn

Upstream: https://github.com/ssandrews/Smoldyn
Version:  2.76 (commit e8b655f, "Documentation updates and set version number to 2.76",
          2026-10-10; upstream did not tag this release)

Imported subset: `source/Smoldyn`, `source/libSteve`, `source/smoldynconfigure.h.in`,
`LICENSE`, `License.txt`, `README.md`, `CHANGELOG.md`. Not imported: NextSubVolume, BioNetGen,
pybind11/python, SmolCrowd, SmolEmulate, wrl2smol, MSVClibs, `source/vcell` (the bridge
provides the VCell mesh/value-provider classes), docs, examples. Removed from the import:
build artefacts `libSteve/testcode`, `libSteve/testcode.dSYM`, `Smoldyn/libsmoldyn.la`.

VCell modifications are local to this repository and are made in commits after the import
commit, so `git diff <import commit> -- smoldyn-2.76` shows all of them. `CMakeLists.txt` is
VCell's own, not upstream's.

## VCell modifications

Upstream does not build or test its `OPTION_VCELL` configuration, so 2.76 needed these local
changes for VCell:

- `CMakeLists.txt`: VCell's own build of `smoldyn_static`. Every source is compiled as C++
  (the VCell declarations in `smoldyn.h` are C++). Exports `VCELL_SMOLDYN_VERSION=276` and
  `SMOLDYN_CXX_LINKAGE`, and sets a default `BNG2_PATH`.
- `libSteve/{List,opengl2,random2,Rn,RnSparse,SimCommand}.h`, `Smoldyn/smoldynfuncs.h`: the
  `extern "C"` blocks are skipped when `SMOLDYN_CXX_LINKAGE` is defined. With everything
  compiled as C++, the other libSteve headers give C++ linkage, and a function's linkage would
  otherwise depend on which header a translation unit reached first.
- `Smoldyn/smoldyn.h`: with `OPTION_VCELL`, include the real libSteve headers before the
  copies of their declarations that `smoldyn.h` carries. Those copies are stale
  (`cmdsuperstruct` has a different layout, `STRCHAR` is 256 rather than 512), so a file that
  included `smoldyn.h` first (e.g. the bridge) disagreed with Smoldyn about struct layouts.
- `Smoldyn/smolcomparts.c`: close the `extern "C"` block around `zlib.h` (the brace was on the
  `#include` line).
- `Smoldyn/smolsim.cpp`, `smolreact.c`, `smolsurface.c`: include `<sstream>` for the VCell code.
- `Smoldyn/smolsurface.c`: pass `index` to `surfsetrate` in the VCell constant-rate branch
  (the signature gained it upstream).
- `Smoldyn/smolfilament.c`: cast `calloc` results (needed as C++).
- `Smoldyn/smoldynfuncs.h`: declare `runSmoldyn`.
- `Smoldyn/smolreact.c`: VCell's mesh-based 0th-order creation (`zeroreactcmptmesh`,
  vcell-fvsolver#24 and #26), carried over from the 2.38 tree with the new `posincompart`
  argument.
- `Smoldyn/smolsurface.c` and the files that look up surfaces or panels by name: VCell's
  surface/panel name hashtables, which replace linear `stringfind` searches (quadratic in the
  number of panels when a VCell mesh's `neighbors` statements are read). Unlike the 2.38 port,
  the tables (`snameindex`, `pnameindex[ps]` in `smoldyn.h`) map names to indices, and
  `surfnameindex`/`panelnameindex` are drop-in replacements for the `stringfind` calls, so
  upstream's function names and signatures, and its default panel names, are unchanged.
- Carried over from VCell's 2.38 tree: `Smoldyn/vcell_smoldyn.cpp` (the `smoldyn_x64` entry
  point, updated for the new `simSetLogging` signature and with the includes it needs) and
  `libSteve/ComplexDataStructures/` (unchanged; used by the gtest suite).
