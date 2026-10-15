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
