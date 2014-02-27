Name:       atruncate
Summary:    Automatic truncation of zero-bytes from end of a file
Version:    1.0.0
Release:    1
Group:      Development/Tools/Other
License:    GPLv2
URL:        https://bitbucket.org/jolla/tools-atruncate
Source0:    %{name}-%{version}.tar.bz2

%description
%{summary}

%prep
%setup -q -n %{name}-%{version}

%build

make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install

%files
%defattr(755,root,root,-)
%{_bindir}/*
