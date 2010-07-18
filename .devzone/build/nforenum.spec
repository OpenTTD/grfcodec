Name:           %{dz_repo}
Version:        3.4.6+r2304M+%{dz_version}
Release:        %{_vendor}%{?suse_version}
Summary:        A format correcter and linter for the NFO language
Group:          Development/Tools
License:        GPLv2+
URL:            http://dev.openttdcoop.org/projects/nforenum
Source0:        nforenum-%{dz_version}.tar

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
make %{?_smp_mflags} UPX= release bundle_src

%install
install -Ds -m0755 nforenum %{buildroot}%{_bindir}/nforenum

%clean

%files
%defattr(-,root,root,-)
%doc CHANGELOG.txt COPYING.txt TODO.txt
%doc doc/
%{_bindir}/nforenum

%changelog
