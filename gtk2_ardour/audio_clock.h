/*
    Copyright (C) 1999 Paul Davis

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#ifndef __audio_clock_h__
#define __audio_clock_h__

#include <map>
#include <vector>

#include <pangomm.h>

#include <gtkmm/alignment.h>
#include <gtkmm/box.h>
#include <gtkmm/menu.h>
#include <gtkmm/label.h>

#include "ardour/ardour.h"
#include "ardour/session_handle.h"

#include "gtkmm2ext/cairo_widget.h"
#include "widgets/ardour_button.h"

namespace ARDOUR {
	class Session;
}

class AudioClock : public CairoWidget, public ARDOUR::SessionHandlePtr
{
	public:
	enum Mode {
		Timecode,
		BBT,
		MinSec,
		Samples
	};

	AudioClock (const std::string& clock_name, bool is_transient, const std::string& widget_name,
	            bool editable, bool follows_playhead, bool duration = false, bool with_info = false,
	            bool accept_on_focus_out = false);
	~AudioClock ();

	Mode mode() const { return _mode; }
	void set_off (bool yn);
	bool off() const { return _off; }
	bool on() const { return !_off; }
	void set_widget_name (const std::string& name);
	void set_active_state (Gtkmm2ext::ActiveState s);
	void set_editable (bool yn);
	void set_corner_radius (double);

	void focus ();

	void set (samplepos_t, bool force = false, ARDOUR::samplecnt_t offset = 0);
	void set_time (Temporal::timepos_t const &, bool force = false, Temporal::timecnt_t const & offset = Temporal::timecnt_t());
	void set_duration (Temporal::timecnt_t const &, bool force = false, Temporal::timecnt_t const & offset = Temporal::timecnt_t());

	void set_from_playhead ();
	void locate ();
	void set_mode (Mode, bool noemit = false);
	void set_bbt_reference (samplepos_t);
	void set_is_duration (bool, Temporal::timepos_t const &);

	void copy_text_to_clipboard () const;

	std::string name() const { return _name; }

	samplepos_t current_time () const;
	Temporal::timepos_t current_position () const;
	Temporal::timecnt_t  current_time_duration () const;
	ARDOUR::samplecnt_t current_sample_duration (samplepos_t pos) const;
	Temporal::Beats current_beat_duration (Temporal::timepos_t const &) const;
	void set_session (ARDOUR::Session *s);
	void set_negative_allowed (bool yn);

	ArdourWidgets::ArdourButton* left_btn () { return &_left_btn; }
	ArdourWidgets::ArdourButton* right_btn () { return &_right_btn; }

	/** Alter cairo scaling during rendering.
	 *
	 * Used by clocks that resize themselves
	 * to fit any given space. Can lead
	 * to font distortion.
	 */
	void set_scale (double x, double y);

	static void print_minsec (samplepos_t, char* buf, size_t bufsize, float sample_rate);

	sigc::signal<void> ValueChanged;
	sigc::signal<void> mode_changed;
	sigc::signal<void> ChangeAborted;

	static sigc::signal<void> ModeChanged;
	static std::vector<AudioClock*> clocks;

	protected:
	void render (Cairo::RefPtr<Cairo::Context> const&, cairo_rectangle_t*);
	bool get_is_duration () const { return is_duration; } ;

	virtual void build_ops_menu ();
	Gtk::Menu  *ops_menu;

	bool on_button_press_event (GdkEventButton *ev);
	bool on_button_release_event(GdkEventButton *ev);

	ArdourWidgets::ArdourButton _left_btn;
	ArdourWidgets::ArdourButton _right_btn;

	private:
	Mode             _mode;
	std::string      _name;
	bool              is_transient;
	bool              is_duration;
	bool              editable;
	/** true if this clock follows the playhead, meaning that certain operations are redundant */
	bool             _follows_playhead;
	bool             _accept_on_focus_out;
	bool             _off;
	int               em_width;
	bool             _edit_by_click_field;
	bool             _negative_allowed;
	bool              edit_is_negative;

	ARDOUR::samplecnt_t       _limit_pos;
	Temporal::timepos_t duration_position;

	Glib::RefPtr<Pango::Layout> _layout;

	bool         _with_info;

	Pango::AttrColor*    editing_attr;
	Pango::AttrColor*    foreground_attr;

	Pango::AttrList normal_attributes;
	Pango::AttrList editing_attributes;

	int first_height;
	int first_width;
	bool style_resets_first;
	int layout_height;
	int layout_width;
	double corner_radius;
	uint32_t font_size;

	static const double info_font_scale_factor;
	static const double separator_height;
	static const double x_leading_padding;

	enum Field {
		Timecode_Hours = 1,
		Timecode_Minutes,
		Timecode_Seconds,
		Timecode_frames,
		MS_Hours,
		MS_Minutes,
		MS_Seconds,
		MS_Milliseconds,
		Bars,
		Beats,
		Ticks,
		S_Samples,
	};

	Field index_to_field (int index) const;

	/* this maps the number of input characters/digits when editing
	   to a cursor position. insert_map[N] = index of character/digit
	   where the cursor should be after N chars/digits. it is
	   mode specific and so it is filled during set_mode().
	*/

	std::vector<int> insert_map;

	bool editing;
	std::string edit_string;
	std::string pre_edit_string;
	std::string input_string;

	samplepos_t bbt_reference_time;
	Temporal::timepos_t last_when;
	bool last_pdelta;
	bool last_sdelta;

	bool dragging;
	double drag_start_y;
	double drag_y;
	double drag_accum;
	Field  drag_field;

	void on_realize ();
	bool on_key_press_event (GdkEventKey *);
	bool on_key_release_event (GdkEventKey *);
	bool on_scroll_event (GdkEventScroll *ev);
	void on_style_changed (const Glib::RefPtr<Gtk::Style>&);
	void on_size_request (Gtk::Requisition* req);
	bool on_motion_notify_event (GdkEventMotion *ev);
	bool on_focus_out_event (GdkEventFocus*);

	void set_slave_info ();
	void set_timecode (samplepos_t, bool);
	void set_bbt (samplepos_t, ARDOUR::samplecnt_t, bool);
	void _set_bbt (Temporal::BBT_Time const &, bool negative);
	void set_minsec (samplepos_t, bool);
	void set_samples (samplepos_t, bool);
	void set_out_of_bounds (bool negative);
	void finish_set (Temporal::timepos_t const &, bool);

	void set_clock_dimensions (Gtk::Requisition&);

	samplepos_t get_sample_step (Field, samplepos_t pos = 0, int dir = 1);

	bool timecode_validate_edit (const std::string&);
	bool bbt_validate_edit (const std::string&);
	bool minsec_validate_edit (const std::string&);

	samplepos_t samples_from_timecode_string (const std::string&) const;
	samplepos_t samples_from_bbt_string (samplepos_t, const std::string&) const;
	samplepos_t sample_duration_from_bbt_string (samplepos_t, const std::string&) const;
	samplepos_t samples_from_minsec_string (const std::string&) const;
	samplepos_t samples_from_audioframes_string (const std::string&) const;

	void session_configuration_changed (std::string);
	void session_property_changed (const PBD::PropertyChange&);

	Field index_to_field () const;

	void start_edit (Field f = Field (0));
	void end_edit (bool);
	void end_edit_relative (bool);
	void edit_next_field ();
	ARDOUR::samplecnt_t parse_as_distance (const std::string&);

	ARDOUR::samplecnt_t parse_as_timecode_distance (const std::string&);
	ARDOUR::samplecnt_t parse_as_minsec_distance (const std::string&);
	ARDOUR::samplecnt_t parse_as_bbt_distance (const std::string&);
	ARDOUR::samplecnt_t parse_as_samples_distance (const std::string&);

	static Temporal::BBT_Offset get_bbt_from_string (std::string const & str);

	void set_font (Pango::FontDescription);
	void set_colors ();
	void show_edit_status (int length);
	int  merge_input_and_edit_string ();
	std::string get_field (Field);

	void drop_focus ();
	void dpi_reset ();

	double bg_r, bg_g, bg_b, bg_a;
	double cursor_r, cursor_g, cursor_b, cursor_a;

	double xscale;
	double yscale;
};

#endif /* __audio_clock_h__ */
