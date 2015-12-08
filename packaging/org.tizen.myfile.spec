
Name:       org.tizen.myfile
Summary:    Myfile Application v1.0
Version:    0.3.42
Release:    1
Group:      Applications
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz

%if "%{?tizen_profile_name}" == "wearable" || "%{?tizen_profile_name}" == "tv"
ExcludeArch: %{arm} %ix86 x86_64
%endif

BuildRequires:  pkgconfig(ecore)
BuildRequires:  pkgconfig(elementary)
BuildRequires:  pkgconfig(evas)
BuildRequires:  pkgconfig(efl-extension)
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(libexif)
BuildRequires:  pkgconfig(notification)
BuildRequires:  pkgconfig(pkgmgr)
BuildRequires:  pkgconfig(pkgmgr-info)
BuildRequires:  pkgconfig(storage)
BuildRequires:  pkgconfig(sqlite3)
BuildRequires:  pkgconfig(capi-appfw-application)
BuildRequires:  pkgconfig(capi-appfw-package-manager)
BuildRequires:  pkgconfig(capi-appfw-preference)
BuildRequires:  pkgconfig(capi-base-utils-i18n)
BuildRequires:  pkgconfig(capi-content-media-content)
BuildRequires:  pkgconfig(capi-content-mime-type)
BuildRequires:  pkgconfig(capi-media-metadata-extractor)
BuildRequires:  pkgconfig(capi-system-device)
BuildRequires:  pkgconfig(capi-system-runtime-info)
BuildRequires:  pkgconfig(capi-system-system-settings)
BuildRequires:  pkgconfig(capi-media-thumbnail-util)
BuildRequires: pkgconfig(libtzplatform-config)

BuildRequires:  cmake
BuildRequires:  edje-bin
BuildRequires:  embryo-bin
BuildRequires:  gettext-devel
BuildRequires:  boost-devel
BuildRequires:  boost-thread
BuildRequires:  boost-system
BuildRequires:  boost-filesystem

%description
Myfile Application v1.0.
%define _smack_domain %{name}
Group:      TO_BE/FILLED_IN
#Requires:   %{name} = %{version}-%{release}

%prep
%setup -q

%define PREFIX    %{TZ_SYS_RO_APP}/org.tizen.myfile
%define RESDIR    "%{TZ_SYS_RO_APP}/org.tizen.myfile/res"
%define DATADIR    "%{TZ_SYS_RO_APP}/org.tizen.myfile/data"
%define XMLDIR      %{TZ_SYS_RO_PACKAGES}

%build
%if 0%{?sec_build_binary_debug_enable}
export CFLAGS="$CFLAGS -DTIZEN_DEBUG_ENABLE"
export CXXFLAGS="$CXXFLAGS -DTIZEN_DEBUG_ENABLE"
export FFLAGS="$FFLAGS -DTIZEN_DEBUG_ENABLE"
%endif

cmake . -DCMAKE_INSTALL_PREFIX="%{PREFIX}" \
	-DCMAKE_DESKTOP_ICON_DIR="%{TZ_SYS_RW_ICONS}/default/small" \
	-DCMAKE_INSTALL_PKG_NAME="%{name}" \
	-DCMAKE_INSTALL_DATA_DIR="%{DATADIR}" \
	-DCMAKE_APP_SHARE_PACKAGES_DIR="%{XMLDIR}"

make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}

if [ ! -d %{buildroot}%{TZ_SYS_RO_APP}/org.tizen.myfile/data ]
then
        mkdir -p %{buildroot}%{TZ_SYS_RO_APP}/org.tizen.myfile/data
fi

if [ ! -f %{buildroot}/usr/apps/org.tizen.myfile/data/.myfile_media.db ]
        rm -rf %{buildroot}%{TZ_SYS_RO_APP}/org.tizen.myfile/data/.myfile_media.db*
then
        sqlite3 %{buildroot}%{TZ_SYS_RO_APP}/org.tizen.myfile/data/.myfile_media.db 'PRAGMA journal_mode = PERSIST;
        CREATE TABLE recent_files(path TEXT, name VARCHAR(256), storage_type INT, thumbnail_path TEXT,primary key (path), unique(path) );'
fi

chmod 660 %{buildroot}%{TZ_SYS_RO_APP}/org.tizen.myfile/data/.myfile_media.db
chmod 660 %{buildroot}%{TZ_SYS_RO_APP}/org.tizen.myfile/data/.myfile_media.db-journal

%make_install

%post
chown -R 5000:5000 %{TZ_SYS_RO_APP}/org.tizen.myfile/data

%postun

%files -n org.tizen.myfile
%manifest org.tizen.myfile.manifest
%defattr(-,root,root,-)
%{TZ_SYS_RO_APP}/org.tizen.myfile/bin/myfile
%{TZ_SYS_RO_APP}/org.tizen.myfile/res/locale/*
%{TZ_SYS_RO_APP}/org.tizen.myfile/res/edje/*
%{TZ_SYS_RO_APP}/org.tizen.myfile/data
%{TZ_SYS_RW_PACKAGES}/org.tizen.myfile.xml
%{TZ_SYS_RO_APP}/org.tizen.myfile/shared/res/*
%{TZ_SYS_RW_ICONS}/default/small/org.tizen.myfile.png
