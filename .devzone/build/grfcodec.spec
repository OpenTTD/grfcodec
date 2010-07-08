%define ver %{echo %dz_version | cut -b2-}

Name:           %{dz_repo}
Version:        0.9.10+r3%{ver}
Release:        %{_vendor}%{?suse_version}
Summary:        A suite of programs to modify Transport Tycoon Deluxe's GRF files
Group:          Development/Tools
License:        GPLv2+
URL:            http://dev.openttdcoop.org/projects/grfcodec/
Source0:        grfcodec-%{dz_version}.tar

BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-buildroot

BuildRequires:  gcc-c++

BuildRequires:  boost-devel
#We need Mercurial for auto version detection:
BuildRequires:  mercurial

%description
A suite of programs to modify Transport Tycoon Deluxe's GRF files.
This program is needed to de-/encode graphic extenions, which you 
need to build OpenGFX.

%prep
%setup -qn %{name}

%build
make %{?_smp_mflags} UPX=

%install
#make install
for file in grfcodec grfdiff grfmerge; do
  install -sD -m0755 $file %{buildroot}%{_bindir}/$file
done

%clean

%files
%defattr(-,root,root,-)
%doc Changelog COPYING grfcodec.txt grftut.txt grf.txt
%{_bindir}/grfcodec
%{_bindir}/grfdiff
%{_bindir}/grfmerge

%changelog
