Name:           %{dz_repo}
Version:        999.%{dz_version}
Release:        %{_vendor}%{?suse_version}
Summary:        A suite of programs to develop NewGRFs
Group:          Development/Tools/Building
License:        GPLv2+
URL:            http://dev.openttdcoop.org/projects/grfcodec
Source0:        %{name}-%{dz_version}.tar

BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-buildroot

BuildRequires:  gcc-c++
BuildRequires:  boost-devel > 1.36
BuildRequires:  libpng-devel

Provides:       nforenum = %{version}
Obsoletes:      nforenum < %{version}

#We need Mercurial for auto version detection: (not needed with source tarball)
BuildRequires:  mercurial

%description
A suite of programs to modify Transport Tycoon Deluxe's GRF files.
Contains GRFCodec for encoding and decoding the actual GRF files,
GRFID for extracting the (unique) NewGRF identifier and NFORenum,
a format correcter and linter for the NFO language. NFO and PCX
or PNG files are encoded to form GRF files.

%prep
%setup -qn %{name}

%build
make %{?_smp_mflags}

%install
make install DESTDIR=%{buildroot} prefix=%{_prefix}

%clean

%files
%defattr(-,root,root,-)
%{_bindir}/grfcodec
%{_bindir}/grfid
%{_bindir}/grfstrip
%{_bindir}/nforenum
%dir %{_datadir}/doc/grfcodec
%doc %{_datadir}/doc/grfcodec/COPYING
%doc %{_datadir}/doc/grfcodec/*.txt
%{_mandir}/man1/grfcodec.1*
%{_mandir}/man1/grfid.1*
%{_mandir}/man1/grfstrip.1*
%{_mandir}/man1/nforenum.1*

%changelog
