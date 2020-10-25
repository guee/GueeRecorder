Summary: 高效率的录屏软件，可以融合多种图像元素，编码为视频文件。
Name: net.guee.recorder
BuildArch: mips64el
Version: 1.0.1
Release: 2
License: GPLv3
Packager: guee@guee.net
Vendor: guee
Group: Unspecified
URL: http://www.loongson.xyz/Articles/Content/18
Requires: mesa-libGL, mesa-libGLU, qt5-qtx11extras, libXinerama, libXfixes, libXcomposite, qt5-qtmultimedia, qt5-qtsvg, qt5-qtimageformats
BuildRoot: %{_topdir}/BUILDROOT
%description
guee recorder
%files
%attr(0777, root, root) "/opt/guee/recorder/GueeRecorder"
%attr(0777, root, root) "/opt/guee/recorder/lib/lisa64/3a3000/libx264.so.161"
%attr(0777, root, root) "/opt/guee/recorder/lib/lisa64/3a3000/libfaac.so.0"
%attr(0777, root, root) "/opt/guee/recorder/lib/lisa64/3a4000/libx264.so.161"
%attr(0777, root, root) "/opt/guee/recorder/lib/lisa64/3a4000/libfaac.so.0"
%attr(0777, root, root) "/opt/guee/recorder/lib/lisa64/libstdc++.so.6"
%attr(0644, root, root) "/usr/share/applications/net.guee.recorder.desktop"
%attr(0644, root, root) "/usr/share/icons/hicolor/scalable/apps/net.guee.recorder.svg"
#attr(0644, root, root) "/usr/share/GueeRecorder/translations/GueeRecorder_ar.qm"
%changelog
