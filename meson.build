project('jacktrip', ['cpp','c'],
		default_options: ['cpp_std=c++17','warning_level=2','optimization=2'])

if get_option('profile') == 'development'
	application_id = 'org.jacktrip.JackTrip.Devel'
	name_suffix = ' (Development Snapshot)'
else
	application_id = 'org.jacktrip.JackTrip'
	name_suffix = ''
endif

qt_version = get_option('qtversion')
if qt_version == ''
	qt_dep = dependency('qt6', modules: ['Core'], required: false)
	if qt_dep.found()
		qt_version = '6'
	else
		qt_version = '5'
	endif
endif
if qt_version == '5' and get_option('nogui') == false and get_option('novs') == false
	error('JackTrip Virtual Studio requires Qt 6.2 or later.')
endif
qt = import('qt' + qt_version)

cmake = import('cmake')

compiler = meson.get_compiler('cpp')

defines = ['-DWAIRTOHUB']
c_defines = []
incdirs = []

if get_option('debug') == false
	defines += ['-DNDEBUG', '-DQT_NO_DEBUG']
	c_defines += ['-DNDEBUG', '-DQT_NO_DEBUG']
endif

build_info = get_option('buildinfo')
git = find_program('git', required: false)
if build_info == '' and git.found()
	git_tags_cmd = run_command(git, 'describe', '--tags', check: false)
	git_hash_cmd = run_command(git, 'rev-parse', '--short', 'HEAD', check: false)
	if git_tags_cmd.returncode() == 0 and git_hash_cmd.returncode() == 0
		git_tags = git_tags_cmd.stdout().strip()
		git_hash = git_hash_cmd.stdout().strip()
		build_info = git_tags + '-' + git_hash
	endif
endif
if build_info != ''
	message('Build info: ' + build_info)
	defines += ['-DJACKTRIP_BUILD_INFO=' + build_info]
endif

src = [	'src/DataProtocol.cpp',
	'src/JackTrip.cpp',
	'src/ProcessPlugin.cpp',
	'src/AudioTester.cpp',
	'src/jacktrip_globals.cpp',
	'src/JackTripWorker.cpp',
	'src/LoopBack.cpp',
	'src/PacketHeader.cpp',
	'src/RingBuffer.cpp',
	'src/JitterBuffer.cpp',
	'src/Regulator.cpp',
	'src/Settings.cpp',
	'src/UdpDataProtocol.cpp',
	'src/UdpHubListener.cpp',
	'src/AudioInterface.cpp',
	'src/Compressor.cpp',
	'src/Limiter.cpp',
	'src/Meter.cpp',
	'src/Monitor.cpp',
	'src/Volume.cpp',
	'src/Tone.cpp',
	'src/StereoToMono.cpp',
	'src/Reverb.cpp',
	'src/main.cpp',
	'src/SslServer.cpp',
	'src/Auth.cpp']

moc_h = ['src/DataProtocol.h',
	'src/JackTrip.h',
	'src/ProcessPlugin.h',
	'src/Meter.h',
	'src/Monitor.h',
	'src/StereoToMono.h',
	'src/Volume.h',
	'src/Tone.h',
	'src/JackTripWorker.h',
	'src/PacketHeader.h',
	'src/Regulator.h',
	'src/Settings.h',
	'src/UdpDataProtocol.h',
	'src/UdpHubListener.h',
	'src/Auth.h',
	'src/SslServer.h']

ui_h = []
qres = []
deps = [dependency('threads')]
link_args = []

subdir('win')
subdir('linux')

jack_dep = dependency('jack', required: get_option('jack'))
if not jack_dep.found()
	if get_option('weakjack') == true
		error('unable to find jack and weakjack requested')
	endif
	defines += '-DNO_JACK'
	c_defines += '-DNO_JACK'
else
	src += 	['src/JackAudioInterface.cpp',
		'src/JMess.cpp',
		'src/Patcher.cpp']
	moc_h += ['src/Patcher.h']
	if get_option('weakjack') == true
		incdirs += include_directories('externals/weakjack')
		src += 'externals/weakjack/weak_libjack.c'
		defines += '-DUSE_WEAK_JACK'
		c_defines += '-DUSE_WEAK_JACK'
		deps += jack_dep.partial_dependency(compile_args: true, includes: true)
		deps += compiler.find_library('dl', required : false)
	else
		deps += jack_dep
	endif
endif

if qt_version == '5'
	deps += dependency('qt5', modules: ['Core', 'Network'], include_type: 'system')
else
	deps += dependency('qt6', modules: ['Core', 'Network'], include_type: 'system')
endif

if get_option('nogui') == true or (get_option('noclassic') == true and get_option('novs') == true)
	# command line only
	defines += '-DNO_GUI'
else
	# include vs and/or classic gui
	if qt_version == '5'
		deps += dependency('qt5', modules: ['Gui', 'Widgets'], include_type: 'system')
	else
		deps += dependency('qt6', modules: ['Gui', 'Widgets'], include_type: 'system')
	endif
	qres = ['src/images/images.qrc']
	src += 'src/UserInterface.cpp'

	if get_option('noclassic') == true
		defines += '-DNO_CLASSIC'
	else
		# support classic mode
		src += [
			'src/gui/qjacktrip.cpp',
			'src/gui/about.cpp',
			'src/gui/messageDialog.cpp',
			'src/gui/textbuf.cpp',
			'src/gui/vuMeter.cpp'
		]
		moc_h += [
			'src/gui/about.h',
			'src/gui/qjacktrip.h',
			'src/gui/messageDialog.h',
			'src/gui/textbuf.h',
			'src/gui/vuMeter.h'
		]
		ui_h += [
			'src/gui/qjacktrip.ui',
			'src/gui/messageDialog.ui',
			'src/gui/about.ui'
		]
	endif

	if get_option('novs') == true
		defines += '-DNO_VS'
	else
		src += [
			'src/vs/virtualstudio.cpp',
			'src/vs/vsAuth.cpp',
			'src/vs/vsApi.cpp',
			'src/vs/vsDeviceCodeFlow.cpp',
			'src/vs/vsDeeplink.cpp',
			'src/vs/vsDevice.cpp',
			'src/vs/vsAudio.cpp',
			'src/vs/vsServerInfo.cpp',
			'src/vs/vsQuickView.cpp',
			'src/vs/vsWebSocket.cpp',
			'src/vs/vsPermissions.cpp',
			'src/vs/vsPinger.cpp',
			'src/vs/vsPing.cpp',
			'src/vs/WebSocketTransport.cpp'
		]
		moc_h += [
			'src/vs/virtualstudio.h',
			'src/vs/vsApi.h',
			'src/vs/vsAuth.h',
			'src/vs/vsDeviceCodeFlow.h',
			'src/vs/vsDeeplink.h',
			'src/vs/vsDevice.h',
			'src/vs/vsAudio.h',
			'src/vs/vsServerInfo.h',
			'src/vs/vsQuickView.h',
			'src/vs/vsWebSocket.h',
			'src/vs/vsPermissions.h',
			'src/vs/vsPinger.h',
			'src/vs/vsPing.h',
			'src/vs/vsQmlClipboard.h',
			'src/vs/JTApplication.h',
			'src/vs/WebSocketTransport.h'
		]

		if host_machine.system() == 'darwin'
			moc_h += ['src/vs/vsMacPermissions.h']
		endif

		if get_option('vsftux') == true or get_option('noclassic') == true
			defines += '-DVS_FTUX'
		endif

		deps += dependency('qt6', modules: ['Core5Compat', 'Quick', 'QuickControls2', 'Qml', 'ShaderTools', 'Svg', 'WebSockets', 'WebEngineCore', 'WebEngineQuick', 'WebChannel'], include_type: 'system')
		qres += ['src/vs/vs.qrc']
	endif

	if get_option('noupdater') == true or host_machine.system() == 'linux'
		defines += '-DNO_UPDATER'
	else
		src += [
			'src/dblsqd/feed.cpp',
			'src/dblsqd/release.cpp',
			'src/dblsqd/semver.cpp',
			'src/dblsqd/update_dialog.cpp'
		]
		moc_h += [
			'src/dblsqd/feed.h',
			'src/dblsqd/update_dialog.h'
		]
		ui_h += ['src/dblsqd/update_dialog.ui']
	endif

	if get_option('nofeedback') == true
		defines += '-DNO_FEEDBACK'
	else
		src += [ 'src/Analyzer.cpp' ]
		moc_h += [ 'src/Analyzer.h' ]
	endif
endif

if get_option('default_library') == 'static'
	# use qmake to get paths for qt libraries and plugins
	# seems like qt module should have a method for this, but it doesn't
	qmake = find_program('qmake', required: true)
	qt_libdir = run_command(qmake, '-query', 'QT_INSTALL_LIBS', check : true).stdout().strip()
	qt_plugindir = run_command(qmake, '-query', 'QT_INSTALL_PLUGINS', check : true).stdout().strip()
	if qt_version == '6'
		# qt6 requires "Bundled*" modules for linking
		deps += dependency('qt6', modules: ['BundledLibpng', 'BundledPcre2', 'BundledHarfbuzz', 'BundledZLIB'], include_type: 'system')
	else
		deps += compiler.find_library('qtpcre2', required : true, dirs : [qt_libdir])
	endif
	if (host_machine.system() == 'linux')
		# linux static
		deps += compiler.find_library('ssl', required : true, dirs : [qt_libdir])
		deps += compiler.find_library('crypto', required : true, dirs : [qt_libdir])
		deps += compiler.find_library('dl', required : true)
		deps += compiler.find_library('glib-2.0', required : true)
		if qt_version == '6'
			# we need a Q_IMPORT_LIBRARY for the openssl backend on linux
			deps += compiler.find_library('qopensslbackend', required : true, dirs : [qt_plugindir+'/tls'])
			src += ['src/QtStaticPlugins.cpp']
		endif
	else
		if (host_machine.system() == 'windows')
			# windows static
			deps += compiler.find_library('bcrypt', required : true)
			deps += compiler.find_library('winmm', required : true)
			deps += compiler.find_library('Crypt32', required : true)
			if qt_version == '6'
				deps += compiler.find_library('Authz', required : true)
			endif
		else
			# mac static
			# this approach fails for universal builds, so we have to just append to link_args
			#deps += dependency('CoreServices', required : true)
			link_args += ['-framework', 'CoreServices']
			link_args += ['-framework', 'CFNetwork']
			link_args += ['-framework', 'AppKit']
			link_args += ['-framework', 'IOKit']
			link_args += ['-framework', 'Security']
			link_args += ['-framework', 'GSS']
			link_args += ['-framework', 'SystemConfiguration']
			deps += dependency('zlib', required : true)
		endif
	endif
endif

# QT_OPENSOURCE should only be defined for open source Qt distribution
if get_option('qtedition') != 'commercial'
	defines += '-DQT_OPENSOURCE'
endif

rtaudio_dep = dependency('rtaudio', required: get_option('rtaudio'))
if rtaudio_dep.found() == true
	defines += '-DRT_AUDIO'
	src += 'src/RtAudioInterface.cpp'
	deps += rtaudio_dep
endif

if rtaudio_dep.found() == false and jack_dep.found() == false
	error('''
	JackTrip requires at least one available audio backend. Install the
	appropriate library or enable the appropriate backends using meson
	configure.''')
endif

if host_machine.system() == 'darwin'
	src += ['src/NoNap.mm']
	# Adding CoreAudio here is a workaround and should be removed
	# when https://github.com/thestk/rtaudio/issues/302 is fixed
	# and arrived in all common package managers
	# Check at 2022-07-30
	apple_dep = dependency('appleframeworks', modules : ['foundation','coreaudio'])
	deps += apple_dep
	add_languages('objcpp')
endif

if host_machine.system() == 'darwin' and get_option('novs') == false and get_option('nogui') == false
	src += ['src/vs/vsMacPermissions.mm']
	apple_av_dep = dependency('appleframeworks', modules : ['avfoundation', 'webkit'])
	deps += apple_av_dep
endif

qres_files = []
if qres.length() > 0
	qres_files = qt.compile_resources(sources: qres)
endif
moc_files = qt.compile_moc(headers: moc_h, extra_args: defines)
ui_files = []
if ui_h.length() > 0
	ui_files = qt.compile_ui(sources: ui_h)
endif

jacktrip = executable('jacktrip', src, qres_files, ui_files, moc_files, include_directories: incdirs, dependencies: deps, link_args: link_args, c_args: c_defines, cpp_args: defines, install: true )

help2man = find_program('help2man', required: false)
if (host_machine.system() == 'linux')
	if help2man.found()
		gzip = find_program('gzip', required: false)
		help2man_opts = [
			'--name="high-quality system for audio network performances"',
			'--no-info',
			'--section=1']
		manfile = custom_target('jacktrip.1',
			output: 'jacktrip.1',
			command: [help2man, help2man_opts, '--output=@OUTPUT@', jacktrip],
			install: not gzip.found(),
			install_dir: get_option('mandir') / 'man1')
		if gzip.found()
			custom_target('jacktrip.1.gz',
				input: manfile,
				output: 'jacktrip.1.gz',
				command: [gzip, '-k', '-f', '@INPUT@'],
				install: true,
				install_dir: get_option('mandir') / 'man1')
		endif
	endif
endif

summary({'JACK': jack_dep.found(),
	'Weak JACK Linking': get_option('weakjack'),
	'RtAudio': rtaudio_dep.found()}, bool_yn: true, section: 'Audio Backends')

summary({'Application ID': application_id,
	'GUI': not get_option('nogui'),
	'WAIR': get_option('wair'),
	'Manpage': help2man.found()}, bool_yn: true, section: 'Configuration')
