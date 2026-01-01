# strans

An input method daemon for CJK text entry on X11.

Inspired by 9front's ktrans. Threads communicate via CSP channels.

## Dependencies

- plan9port
- gtk+-3.0 (optional, for GTK IM module)

## Build

	make
	cd xim && make                          # XIM adapter
	cd gtk && make docker && make install   # GTK IM module

## Run

	./strans map font &

For XIM apps:

	./xim/strans-xim &
	XMODIFIERS=@im=strans xterm

For GTK apps:

	GTK_IM_MODULE=strans gedit

## Usage

Switch input modes with Ctrl + key:

	N  Hiragana
	K  Katakana
	S  Hangul
	T  English
	V  Vietnamese (Telex)
	E  Emoji

Type romanized input. Select candidates with 1-9 or arrow keys.
Tab or Enter to commit.

## Architecture

Four threads communicate via CSP channels:

- [imthread](strans.c#L271): keystroke processing, transliteration
- [dictthread](dict.c#L38): dictionary lookup
- [drawthread](win.c#L133): preedit window rendering
- [srvthread](srv.c#L42): IPC via unix socket

Adapters (strans-xim, im-strans.so) bridge X11/GTK events.

## Files

	strans.c    input method engine
	dict.c      dictionary queries
	win.c       xcb window management
	font.c      truetype rendering (stb_truetype)
	map/        transliteration tables
	font/       bundled CJK fonts

## References

- https://git.9front.org/plan9front/plan9front/HEAD/info.html
- https://np.mkv.li/kor/9front-ktrans-%ED%95%9C%EA%B8%80-%EB%98%91%EB%B0%94%EB%A1%9C-%EB%A7%8C%EB%93%A4%EA%B8%B0/
- https://en.wikipedia.org/wiki/Telex_(input_method)
- https://gist.github.com/hieuthi/0f5adb7d3f79e7fb67e0e499004bf558
