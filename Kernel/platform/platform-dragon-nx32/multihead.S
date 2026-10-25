;
; dispatches vt calls to different screen routines
;


	; Chekc if we need it special location
	.data
	.export	_curtty

_curtty:
	.byte	0

	.code		; TODO review

	.export _clear_across
_clear_across:
	lda	_curtty
	lbeq	_m6847_clear_across
	deca
	lbeq	_crt9128_cacross
	jmp	_vc_clear_across

	.export _clear_lines
_clear_lines:
	lda	_curtty
	lbeq	_m6847_clear_lines
	deca
	lbeq	_crt9128_clines
	jmp	_vc_clear_lines

	.export _scroll_up
_scroll_up:
	lda	_curtty
	lbeq	_m6847_scroll_up
	deca
	lbeq	_crt9128_scrup
	jmp	_vc_scroll_up

	.export _scroll_down
_scroll_down:
	lda	_curtty
	lbeq	_m6847_scroll_down
	deca
	lbeq	_crt9128_scrdown
	jmp	_vc_scroll_down

	.export _plot_char
_plot_char:
	lda	_curtty
	lbeq	_m6847_plot_char
	deca
	lbeq	_crt9128_plot_char
	jmp	_vc_plot_char

	.export _cursor_off
_cursor_off:
	lda	_curtty
	lbeq	_m6847_cursor_off
	deca
	lbeq	_crt9128_coff
	jmp	_vc_cursor_off

	.export _cursor_on
_cursor_on:
	lda	_curtty
	lbeq	_m6847_cursor_on
	deca
	lbeq	_crt9128_con
	jmp	_vc_cursor_on

	.export _cursor_disable
_cursor_disable:
	rts

	.export _vtattr_notify
_vtattr_notify:
	lda	_curtty
	lbeq	_m6847_vtattr_notify
	deca
	lbeq	_crt9128_vtattr_notify
	jmp	_vc_vtattr_notify

	.export _video_cmd
_video_cmd:
	tst	_curtty
	lbeq	_m6847_video_cmd
	rts

	.export _video_read
_video_read:
	tst	_curtty
	lbeq	_m6847_video_read
	rts

	.export _video_write
_video_write:
	tst	_curtty
	lbeq	_m6847_video_write
	rts

