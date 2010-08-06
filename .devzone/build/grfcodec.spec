Name:           %{dz_repo}
Version:        0.9.10+r2294+%{dz_version}
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
#Targets release and bundle_src are needed for DevZone only
make %{?_smp_mflags} release bundle_src

%install
make install INSTALL_DIR=%{buildroot}

%clean

%files
%defattr(-,root,root,-)
%{_bindir}/grfcodec
%{_bindir}/grfdiff
%{_bindir}/grfid
%{_bindir}/grfmerge
%dir %{_datadir}/doc/grfcodec
%doc %{_datadir}/doc/grfcodec/CHANGELOG.txt
%doc %{_datadir}/doc/grfcodec/COPYING.txt
%doc %{_datadir}/doc/grfcodec/grf.txt
%doc %{_datadir}/doc/grfcodec/grfcodec.txt
%doc %{_datadir}/doc/grfcodec/grftut.txt
%doc %{_mandir}/man1/grfcodec.1.gz
%doc %{_mandir}/man1/grfdiff.1.gz
%doc %{_mandir}/man1/grfid.1.gz
%doc %{_mandir}/man1/grfmerge.1.gz

%changelog
