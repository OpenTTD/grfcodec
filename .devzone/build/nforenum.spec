Name:           %{dz_repo}
Version:        3.4.6+r2304M+%{dz_version}
Release:        %{_vendor}%{?suse_version}
Summary:        A format correcter and linter for the NFO language
Group:          Development/Tools
License:        GPLv2+
URL:            http://dev.openttdcoop.org/projects/nforenum
Source0:        %{name}-%{dz_version}.tar

BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-buildroot

BuildRequires:  gcc-c++
BuildRequires:  boost-devel
#We need Mercurial for auto version detection:
BuildRequires:  mercurial

%description
A format correcter and linter for the NFO language. NFO is used for
graphic extensions of the Transport Tycoon games TTDPatch and
OpenTTD, this rpm is dedicated to OpenGFX, the free replacement to
make it possible using OpenTTD without the need of the original
graphics.

%prep
%setup -qn %{name}

%build
make %{?_smp_mflags}
#remove the very outdated CZ docs:
rm doc/*cz.txt

%install
make install INSTALL_DIR=%{buildroot}

%clean

%files
%defattr(-,root,root,-)
%{_bindir}/nforenum
%dir %{_datadir}/doc/nforenum
%doc %{_datadir}/doc/nforenum/CHANGELOG.txt
%doc %{_datadir}/doc/nforenum/COPYING.txt
%doc %{_datadir}/doc/nforenum/AUTO_CORRECT.en.txt
%doc %{_datadir}/doc/nforenum/COMMANDS.en.txt
%doc %{_datadir}/doc/nforenum/README.en.txt
%doc %{_datadir}/doc/nforenum/README.RPN.en.txt
%doc %{_datadir}/doc/nforenum/SANITY.en.txt
%doc %{_mandir}/man1/nforenum.1.gz

%changelog
