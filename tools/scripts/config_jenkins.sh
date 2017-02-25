#! /bin/sh

case $1 in
	debian_unstable*)
		cmake ../src \
			-DUSE_CPP14=True \
			-DEXTRA_WARNINGS=True \
			-DENABLE_BLASQ=True \
			-DENABLE_BLASQ_SPEGNERSI=False \
			-DENABLE_AZOTH_WOODPECKER=False \
			-DENABLE_OTLOZHU=True \
			-DENABLE_OTLOZHU_SYNC=False \
			-DENABLE_SYNCER=False \
			-DENABLE_MUSICZOMBIE=True \
			-DENABLE_AZOTH_OTROID=True \
			-DENABLE_AZOTH_VELVETBIRD=True \
			-DENABLE_AZOTH_ZHEET=True \
			-DENABLE_LMP_MTPSYNC=True \
			-DENABLE_LMP_JOS=True \
			-DENABLE_POPISHU=True \
			-DENABLE_VROOBY_UDISKS2=True \
			-DENABLE_MEDIACALLS=False \
			-DENABLE_POLEEMERY=False \
			-DENABLE_AGGREGATOR_WEBACCESS=False \
			-DENABLE_OTZERKALU=True \
			-DENABLE_AZOTH_ASTRALITY=False \
			-DENABLE_ELEEMINATOR=False \
			-DENABLE_POSHUKU_DCAC_TESTS=False \
			-DENABLE_XPROXY=True \
			-DENABLE_FONTIAC=True ;;
	debian_cpp14*)
		CC="/usr/bin/clang" CXX="/usr/bin/clang++" cmake ../src \
			-DUSE_CPP14=True \
			-DENABLE_BLASQ=True \
			-DENABLE_BLASQ_SPEGNERSI=False \
			-DENABLE_AZOTH_WOODPECKER=False \
			-DENABLE_OTLOZHU=True \
			-DENABLE_OTLOZHU_SYNC=False \
			-DENABLE_SYNCER=False \
			-DENABLE_MUSICZOMBIE=False \
			-DENABLE_AZOTH_OTROID=True \
			-DENABLE_AZOTH_VELVETBIRD=True \
			-DENABLE_AZOTH_ZHEET=True \
			-DENABLE_LMP_MTPSYNC=True \
			-DENABLE_LMP_JOS=True \
			-DENABLE_POPISHU=True \
			-DENABLE_VROOBY_UDISKS2=True \
			-DENABLE_QTMULTIMEDIA_KLUDGE=True \
			-DENABLE_POLEEMERY=False \
			-DENABLE_AGGREGATOR_WEBACCESS=False \
			-DENABLE_OTZERKALU=True \
			-DENABLE_AZOTH_ASTRALITY=False \
			-DENABLE_POSHUKU_DCAC_TESTS=False \
			-DENABLE_ELEEMINATOR=False ;;
	fedora_23*)
		cmake ../src \
			-DLIB_SUFFIX=64 \
			-DUSE_CPP14=True \
			-DCMAKE_CXX_FLAGS="-pipe -Wall -Werror=format-security -fexceptions --param=ssp-buffer-size=4 -fasynchronous-unwind-tables" \
			-DSTRICT_LICENSING=True \
			-DWITH_DBUS_LOADERS=True \
			-DWITH_PCRE=True \
			-DWITH_QWT=True \
			-DUSE_LIBTIDY_HTML5=False \
			-DENABLE_ADVANCEDNOTIFICATIONS=True \
			-DENABLE_AGGREGATOR=True \
			-DENABLE_AGGREGATOR_WEBACCESS=True \
			-DENABLE_AUSCRIE=True \
			-DENABLE_AZOTH=True \
			-DENABLE_AZOTH_ABBREV=True \
			-DENABLE_AZOTH_ACETAMIDE=True \
			-DENABLE_AZOTH_ASTRALITY=True \
			-DENABLE_AZOTH_AUTOPASTE=True \
			-DENABLE_AZOTH_MUCOMMANDS=True \
			-DENABLE_AZOTH_MURM=True \
			-DENABLE_AZOTH_OTROID=True \
			-DENABLE_AZOTH_SARIN=False \
			-DENABLE_AZOTH_SHX=True \
			-DENABLE_AZOTH_TRACOLOR=True \
			-DENABLE_AZOTH_VELVETBIRD=True \
			-DENABLE_AZOTH_WOODPECKER=False \
			-DENABLE_AZOTH_ZHEET=False \
			-DENABLE_BLACKDASH=False \
			-DENABLE_BLASQ=True \
			-DENABLE_BLASQ_SPEGNERSI=False \
			-DENABLE_BLOGIQUE=True \
			-DENABLE_CERTMGR=True \
			-DENABLE_CHOROID=True \
			-DENABLE_CPULOAD=True \
			-DENABLE_DEVMON=True \
			-DENABLE_DLNIWE=False \
			-DENABLE_DOLOZHEE=True \
			-DENABLE_DUMBEEP=True \
			-DDUMBEEP_WITH_PHONON=True \
			-DENABLE_ELEEMINATOR=True \
			-DENABLE_FENET=True \
			-DENABLE_FONTIAC=True \
			-DENABLE_GACTS=True \
			-DWITH_GACTS_BUNDLED_QXT=True \
			-DENABLE_GLANCE=True \
			-DENABLE_GMAILNOTIFIER=True \
			-DENABLE_HARBINGER=True \
			-DENABLE_HTTHARE=True \
			-DENABLE_HOTSENSORS=True \
			-DENABLE_HOTSTREAMS=True \
			-DENABLE_HTTHARE=True \
			-DENABLE_IMGASTE=True \
			-DENABLE_INTERMUTKO=True \
			-DENABLE_KBSWITCH=True \
			-DENABLE_KNOWHOW=True \
			-DENABLE_KRIGSTASK=True \
			-DENABLE_LACKMAN=True \
			-DENABLE_LADS=False \
			-DENABLE_LASTFMSCROBBLE=True \
			-DENABLE_LAUGHTY=True \
			-DENABLE_LAUNCHY=True \
			-DENABLE_LEMON=True \
			-DENABLE_LHTR=True \
			-DWITH_LHTR_HTML=True \
			-DENABLE_LIZNOO=True \
			-DENABLE_LMP=True \
			-DUSE_GSTREAMER_10=True \
			-DENABLE_LMP_BRAINSLUGZ=True \
			-DENABLE_LMP_FRADJ=True \
			-DENABLE_LMP_GRAFFITI=True \
			-DENABLE_LMP_LIBGUESS=True \
			-DENABLE_LMP_MPRIS=True \
			-DENABLE_LMP_MTPSYNC=True \
			-DENABLE_LMP_POTORCHU=False \
			-DENABLE_MEDIACALLS=False \
			-DENABLE_MELLONETRAY=True \
			-DENABLE_MONOCLE=True \
			-DENABLE_MONOCLE_MU=False \
			-DENABLE_MUSICZOMBIE=True \
			-DWITH_MUSICZOMBIE_CHROMAPRINT=False \
			-DENABLE_NACHEKU=True \
			-DENABLE_NETSTOREMANAGER=True \
			-DENABLE_NETSTOREMANAGER_DROPBOX=True \
			-DENABLE_NETSTOREMANAGER_GOOGLEDRIVE=True \
			-DENABLE_NEWLIFE=True \
			-DENABLE_OORONEE=True \
			-DENABLE_OTLOZHU=True \
			-DENABLE_OTLOZHU_SYNC=False \
			-DENABLE_PINTAB=True \
			-DENABLE_POLEEMERY=False \
			-DENABLE_POGOOGLUE=True \
			-DENABLE_POPISHU=True \
			-DENABLE_POSHUKU=True \
			-DENABLE_IDN=True \
			-DENABLE_POSHUKU_AUTOSEARCH=True \
			-DENABLE_POSHUKU_DCAC=True \
			-DENABLE_POSHUKU_DCAC_TESTS=False \
			-DENABLE_POSHUKU_QRD=True \
			-DENABLE_POSHUKU_SPEEDDIAL=True \
			-DENABLE_QROSP=False \
			-DENABLE_SB2=True \
			-DENABLE_SCROBLIBRE=True \
			-DENABLE_SECMAN=True \
			-DENABLE_SNAILS=False \
			-DENABLE_SYNCER=False \
			-DENABLE_TABSESSMANAGER=True \
			-DENABLE_TABSLIST=True \
			-DENABLE_TEXTOGROOSE=False \
			-DENABLE_TORRENT=True \
			-DENABLE_BITTORRENT_GEOIP=True \
			-DENABLE_TOUCHSTREAMS=True \
			-DENABLE_TPI=True \
			-DENABLE_TWIFEE=False \
			-DENABLE_VROOBY=True \
			-DENABLE_XPROXY=True \
			-DENABLE_ZALIL=True ;;
	opensuse_factory*)
		cmake ../src -DLIB_SUFFIX=64 \
			-DUSE_CPP14=True \
			-DEXTRA_WARNINGS=True \
			-DCMAKE_INSTALL_PREFIX=/usr \
			-DSTRICT_LICENSING=True \
			-DENABLE_ADVANCEDNOTIFICATIONS=True \
			-DENABLE_AGGREGATOR=True \
			-DENABLE_AGGREGATOR_WEBACCESS=True \
			-DENABLE_AUSCRIE=True \
			-DENABLE_AZOTH=True \
			-DENABLE_AZOTH_ACETAMIDE=True \
			-DENABLE_AZOTH_ASTRALITY=True \
			-DENABLE_AZOTH_OTROID=True \
			-DENABLE_AZOTH_SHX=True \
			-DENABLE_AZOTH_VELVETBIRD=True \
			-DENABLE_AZOTH_WOODPECKER=True \
			-DENABLE_AZOTH_ZHEET=False \
			-DENABLE_MEDIACALLS=False \
			-DENABLE_BLACKDASH=False \
			-DENABLE_BLASQ=True \
			-DENABLE_BLASQ_VANGOG=True \
			-DENABLE_BLOGIQUE=True \
			-DENABLE_CERTMGR=True \
			-DENABLE_CHOROID=False \
			-DENABLE_CPULOAD=True \
			-DENABLE_DEVMON=True \
			-DENABLE_DLNIWE=False \
			-DENABLE_DOLOZHEE=True \
			-DENABLE_DUMBEEP=True \
			-DDUMBEEP_WITH_PHONON=True \
			-DENABLE_GACTS=True \
			-DWITH_GACTS_BUNDLED_QXT=False \
			-DENABLE_GLANCE=True \
			-DENABLE_GMAILNOTIFIER=True \
			-DENABLE_HARBINGER=True \
			-DENABLE_HOTSENSORS=True \
			-DENABLE_HOTSTREAMS=True \
			-DENABLE_HTTHARE=True \
			-DENABLE_IMGASTE=True \
			-DENABLE_KBSWITCH=True \
			-DENABLE_KNOWHOW=True \
			-DENABLE_KRIGSTASK=True \
			-DENABLE_LACKMAN=True \
			-DENABLE_LADS=False \
			-DENABLE_LASTFMSCROBBLE=True \
			-DENABLE_LAUGHTY=True \
			-DENABLE_LAUNCHY=True \
			-DENABLE_LEMON=True \
			-DENABLE_LHTR=True \
			-DWITH_LHTR_HTML=True \
			-DENABLE_LIZNOO=True \
			-DENABLE_LMP=True \
			-DENABLE_LMP_GRAFFITI=True \
			-DENABLE_LMP_LIBGUESS=True \
			-DENABLE_LMP_MPRIS=True \
			-DENABLE_LMP_MTPSYNC=True \
			-DUSE_GSTREAMER_10=True \
			-DENABLE_MELLONETRAY=True \
			-DENABLE_MONOCLE=True \
			-DENABLE_MONOCLE_MU=False \
			-DENABLE_MUSICZOMBIE=True \
			-DWITH_MUSICZOMBIE_CHROMAPRINT=True \
			-DENABLE_NACHEKU=False \
			-DENABLE_NETSTOREMANAGER=True \
			-DENABLE_NEWLIFE=True \
			-DENABLE_OORONEE=True \
			-DENABLE_OTLOZHU=True \
			-DENABLE_OTLOZHU_SYNC=False \
			-DENABLE_PINTAB=True \
			-DENABLE_POGOOGLUE=True \
			-DENABLE_POLEEMERY=False \
			-DENABLE_POPISHU=True \
			-DENABLE_POSHUKU_AUTOSEARCH=True \
			-DENABLE_QROSP=True \
			-DENABLE_SB2=True \
			-DENABLE_SCROBLIBRE=True \
			-DENABLE_SECMAN=True \
			-DENABLE_SNAILS=False \
			-DENABLE_SYNCER=False \
			-DENABLE_TABSESSMANAGER=True \
			-DENABLE_TABSLIST=True \
			-DENABLE_TEXTOGROOSE=True \
			-DENABLE_TORRENT=True \
			-DENABLE_BITTORRENT_GEOIP=True \
			-DENABLE_TOUCHSTREAMS=True \
			-DENABLE_TPI=True \
			-DENABLE_TWIFEE=False \
			-DENABLE_VROOBY=True \
			-DENABLE_ELEEMINATOR=True \
			-DENABLE_XPROXY=True \
			-DENABLE_FONTIAC=True ;;
	opensuse_factory_qt57*)
		cmake ../src -DUSE_QT5=True -DLIB_SUFFIX=64 -DUSE_CPP14=True \
			'-DCMAKE_CXX_FLAGS=-fmessage-length=0 -grecord-gcc-switches -O2 -Wall -D_FORTIFY_SOURCE=2 -fstack-protector -funwind-tables -fasynchronous-unwind-tables -g -Doverride=' \
			-DCMAKE_INSTALL_PREFIX=/usr -DSTRICT_LICENSING=True \
			-DWITH_DBUS_LOADERS=False \
			-DWITH_PCRE=True \
			-DWITH_QWT=True \
			-DENABLE_ADVANCEDNOTIFICATIONS=True \
			-DENABLE_AGGREGATOR=True \
			-DENABLE_AGGREGATOR_WEBACCESS=True \
			-DENABLE_AUSCRIE=True \
			-DENABLE_AZOTH=True \
			-DENABLE_AZOTH_ABBREV=True \
			-DENABLE_AZOTH_ACETAMIDE=True \
			-DENABLE_AZOTH_ASTRALITY=False \
			-DENABLE_AZOTH_AUTOPASTE=True \
			-DENABLE_AZOTH_MUCOMMANDS=True \
			-DENABLE_AZOTH_MURM=True \
			-DENABLE_AZOTH_OTROID=True \
			-DENABLE_AZOTH_SARIN=False \
			-DENABLE_AZOTH_SHX=True \
			-DENABLE_AZOTH_TRACOLOR=False \
			-DENABLE_AZOTH_VELVETBIRD=True \
			-DENABLE_AZOTH_WOODPECKER=False \
			-DENABLE_AZOTH_ZHEET=False \
			-DENABLE_MEDIACALLS=False \
			-DENABLE_BLACKDASH=False \
			-DENABLE_BLASQ=True \
			-DENABLE_BLASQ_VANGOG=True \
			-DENABLE_BLASQ_SPEGNERSI=False \
			-DENABLE_BLOGIQUE=True \
			-DENABLE_CERTMGR=True \
			-DENABLE_CHOROID=True \
			-DENABLE_CPULOAD=True \
			-DENABLE_DEVMON=True \
			-DENABLE_DLNIWE=False \
			-DENABLE_DOLOZHEE=True \
			-DENABLE_DUMBEEP=True \
			-DDUMBEEP_WITH_PHONON=False \
			-DENABLE_ELEEMINATOR=True \
			-DENABLE_FENET=True \
			-DENABLE_FONTIAC=False \
			-DENABLE_GACTS=True \
			-DWITH_GACTS_BUNDLED_QXT=True \
			-DENABLE_GLANCE=True \
			-DENABLE_GMAILNOTIFIER=True \
			-DENABLE_HARBINGER=False \
			-DENABLE_HOTSENSORS=True \
			-DENABLE_HOTSTREAMS=True \
			-DENABLE_HTTHARE=True \
			-DENABLE_IMGASTE=True \
			-DENABLE_INTERMUTKO=True \
			-DENABLE_KBSWITCH=True \
			-DENABLE_KNOWHOW=True \
			-DENABLE_KRIGSTASK=True \
			-DENABLE_LACKMAN=True \
			-DENABLE_LADS=False \
			-DENABLE_LASTFMSCROBBLE=True \
			-DENABLE_LAUGHTY=True \
			-DENABLE_LAUNCHY=True \
			-DENABLE_LEMON=True \
			-DENABLE_LHTR=True \
			-DWITH_LHTR_HTML=True \
			-DENABLE_LIZNOO=True \
			-DENABLE_LMP=True \
			-DENABLE_LMP_BRAINSLUGZ=True \
			-DENABLE_LMP_FRADJ=False \
			-DENABLE_LMP_GRAFFITI=True \
			-DENABLE_LMP_HTTSTREAM=True \
			-DENABLE_LMP_LIBGUESS=True \
			-DENABLE_LMP_MPRIS=True \
			-DENABLE_LMP_MTPSYNC=True \
			-DENABLE_LMP_POTORCHU=True \
			-DUSE_GSTREAMER_10=True \
			-DENABLE_MELLONETRAY=True \
			-DENABLE_MONOCLE=True \
			-DENABLE_MONOCLE_MU=False \
			-DENABLE_MONOCLE_PDF=True \
			-DENABLE_MONOCLE_POSTRUS=True \
			-DENABLE_MUSICZOMBIE=True \
			-DWITH_MUSICZOMBIE_CHROMAPRINT=False \
			-DENABLE_NACHEKU=False \
			-DENABLE_NETSTOREMANAGER=True \
			-DENABLE_NETSTOREMANAGER_DROPBOX=True \
			-DENABLE_NETSTOREMANAGER_GOOGLEDRIVE=True \
			-DENABLE_NEWLIFE=True \
			-DENABLE_OORONEE=True \
			-DENABLE_OTLOZHU=False \
			-DENABLE_PINTAB=True \
			-DENABLE_POGOOGLUE=True \
			-DENABLE_POLEEMERY=True \
			-DENABLE_POPISHU=False \
			-DENABLE_POSHUKU=True \
			-DENABLE_IDN=True \
			-DENABLE_POSHUKU_AUTOSEARCH=True \
			-DENABLE_POSHUKU_DCAC=True \
			-DENABLE_POSHUKU_QRD=True \
			-DENABLE_POSHUKU_SPEEDDIAL=True \
			-DENABLE_QROSP=False \
			-DENABLE_SB2=True \
			-DENABLE_SCROBLIBRE=True \
			-DENABLE_SECMAN=True \
			-DENABLE_SHELLOPEN=False \
			-DENABLE_SNAILS=True \
			-DENABLE_SYNCER=False \
			-DENABLE_TABSESSMANAGER=True \
			-DENABLE_TABSLIST=True \
			-DENABLE_TEXTOGROOSE=True \
			-DENABLE_TORRENT=True \
			-DENABLE_BITTORRENT_GEOIP=True \
			-DENABLE_TOUCHSTREAMS=True \
			-DENABLE_TPI=True \
			-DENABLE_TWIFEE=False \
			-DENABLE_VROOBY=True \
			-DENABLE_WKPLUGINS=False \
			-DENABLE_XPROXY=True \
			-DENABLE_ZALIL=True ;;
esac
