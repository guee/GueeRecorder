Summary: 高效率的录屏软件，可以融合多种图像元素，编码为视频文件。
Name: net.guee.recorder
BuildArch: mips64el
Version: 1.0.1
Release: 0
License: GPLv3
Packager: guee@guee.net
Vendor: guee
Group: Unspecified
URL: http://www.loongson.xyz/Articles/Content/18
Requires: mesa-libGL, mesa-libGLU, qt5-qtx11extras, libXinerama, libXfixes, libXcomposite, qt5-qtmultimedia
BuildRoot: %{_topdir}/BUILDROOT
%description
guee recorder
%files
%attr(0777, root, root) "/usr/bin/GueeRecorder"
%attr(0777, root, root) "/usr/lib64/libx264.so.161"
%attr(0777, root, root) "/usr/lib64/libfaac.so.0"
%attr(0644, root, root) "/usr/share/applications/net.guee.recorder.desktop"
%attr(0644, root, root) "/usr/share/icons/hicolor/16x16/apps/net.guee.recorder.png"
%attr(0644, root, root) "/usr/share/icons/hicolor/24x24/apps/net.guee.recorder.png"
%attr(0644, root, root) "/usr/share/icons/hicolor/32x32/apps/net.guee.recorder.png"
%attr(0644, root, root) "/usr/share/icons/hicolor/48x48/apps/net.guee.recorder.png"
%attr(0644, root, root) "/usr/share/icons/hicolor/64x64/apps/net.guee.recorder.png"
%attr(0644, root, root) "/usr/share/icons/hicolor/128x128/apps/net.guee.recorder.png"
%attr(0644, root, root) "/usr/share/icons/hicolor/256x256/apps/net.guee.recorder.png"
%attr(0644, root, root) "/usr/share/icons/hicolor/512x512/apps/net.guee.recorder.png"
#attr(0644, root, root) "/usr/share/GueeRecorder/translations/GueeRecorder_ar.qm"
%changelog
