Name: greed
Version: 3.7
Release: 1
URL: http://www.catb.org/~esr/greed/
Source: %{name}-%{version}.tar.gz
License: BSD
Group: Games
Summary: the strategy game of Greed
BuildRoot: %{_tmppath}/%{name}-root
XBS-Destinations: freshmeat

%description
The strategy game of Greed.  Try to eat as much as possible of the board
before munching yourself into a corner.

%prep
%setup -q

%build
make %{?_smp_mflags} greed greed.6

%install
[ "$RPM_BUILD_ROOT" -a "$RPM_BUILD_ROOT" != / ] && rm -rf "$RPM_BUILD_ROOT"
mkdir -p "$RPM_BUILD_ROOT"%{_bindir}
mkdir -p "$RPM_BUILD_ROOT"%{_mandir}/man6/
cp greed "$RPM_BUILD_ROOT"%{_bindir}
cp greed.6 "$RPM_BUILD_ROOT"%{_mandir}/man6/

%clean
[ "$RPM_BUILD_ROOT" -a "$RPM_BUILD_ROOT" != / ] && rm -rf "$RPM_BUILD_ROOT"

%files
%doc README COPYING
%defattr(-,root,root,-)
%{_mandir}/man6/greed.6*
%{_bindir}/greed

%changelog
* Wed Oct 20 2010 Eric S. Raymond <esr@snark.thyrsus.com> 3.7-1
- Clean up C for modern POSIX and C99-conformant environments.
- License changed to BSD.

* Mon Dec 29 2003 Eric S. Raymond <esr@snark.thyrsus.com> 3.6-1
- Source RPMS no longer depend on --define myversion.

