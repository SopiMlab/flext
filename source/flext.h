/* 

flext - C++ layer for Max/MSP and pd (pure data) externals

Copyright (c) 2001,2002 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#ifndef __FLEXT_H
#define __FLEXT_H

#define FLEXT_VERSION 102

#include <flbase.h> 


// a few type definitions for the lazy

#define F float 
#define D double
#define I int
#define L long
#define C char
#define BL bool
#define V void
#define FI t_flint


// === flext_base ==================================================

// message only base object


class flext_base:
	public flext_obj
{
	FLEXT_HEADER(flext_base,flext_obj)
	
public:

// ---  compatibility mode ----------------------------------------

	// if set flext allows only operations valid for all platforms
	// true by default!
	static BL compatibility;  


// --- inheritable virtual methods --------------------------------

	// called on "help" message: should post some text
	virtual V m_help();
	
	// called on patcher load (Max/MSP only)
	virtual V m_loadbang() {}

	// quickhelp for inlets/outlets (Max/MSP only)
	virtual V m_assist(L /*msg*/,L /*arg*/,C * /*s*/) {}


// --- buffer/array stuff -----------------------------------------	

	// class for platform independent buffer handling
	class buffer
	{
	public:
		// construct buffer 
		// delayed = true: needs another Set(NULL) to initialize the buffer 
		//                 (in Max the externals are often created prior to the buffer objects)
		buffer(t_symbol *s = NULL,BL delayed = false);
		
		~buffer();
		
		// set to another buffer 
		// nameonly = true: sets name only, no buffer data 
		I Set(t_symbol *s = NULL,BL nameonly = false);
		
		// dirty buffer content
		// refr = true: forces immediate graphics refresh
		V Dirty(BL refr = false);
		
		// get symbol (or literal name) of buffer 
		t_symbol *Symbol() const { return sym; }
		const C *Name() const { return sym?sym->s_name:""; }
		
		// get pointer to buffer, channel and frame count
		F *Data() { return data; }
		I Channels() const { return chns; }
		I Frames() const { return frames; }
		
		// graphic auto refresh interval
		V SetRefrIntv(F intv);

	protected:
		t_symbol *sym;
		F *data;
		I chns,frames;
#ifdef PD
		F interval;
		BL isdirty,ticking;
		t_clock *tick;

	private:
		static V cb_tick(buffer *b);
#endif
	};


// --- inlet/outlet stuff -----------------------------------------	

	// define inlets/outlets - all (also default) inlets must be defined
	V add_in_def() { AddInlet(xlet::tp_def,1); }
	V add_in_float(I m = 1) { AddInlet(xlet::tp_float,m); }
	V add_in_flint(I m = 1) { AddInlet(xlet::tp_flint,m); }
	V add_in_symbol(I m = 1) { AddInlet(xlet::tp_sym,m); }
	V add_in_list(I m = 1) { AddInlet(xlet::tp_list,m); }  // via proxy
	V add_in_anything(I m = 1) { AddInlet(xlet::tp_any,m); } // via proxy
	
	V add_out_float(I m = 1) { AddOutlet(xlet::tp_float,m); }
	V add_out_flint(I m = 1) { AddOutlet(xlet::tp_flint,m); }
	V add_out_symbol(I m = 1) { AddOutlet(xlet::tp_sym,m); }
	V add_out_list(I m = 1) { AddOutlet(xlet::tp_list,m); }
	V add_out_anything(I m = 1) { AddOutlet(xlet::tp_any,m); }
	
	// must be called to actually set up the defined inlets/outlets 
	// only ONCE!!!
	BL setup_inout(); 

	I cnt_in() const { return incnt; }
	I cnt_out() const { return outcnt; }
	I cnt_insig() const { return insigs; }
	I cnt_outsig() const { return outsigs; }

	// get pointer _after_wards	
	t_outlet *get_out(I ix) { return (outlets && ix < outcnt)?outlets[ix]:NULL; }

	// output messages (n starts with 0)
	V to_out_float(t_outlet *o,F f) { outlet_float(o,f); }
	V to_out_float(I n,F f) { t_outlet *o = get_out(n); if(o) to_out_float(o,f); }
	V to_out_flint(t_outlet *o,FI f) { outlet_flint(o,f); }
	V to_out_flint(I n,FI f) { t_outlet *o = get_out(n); if(o) to_out_flint(o,f); }
	V to_out_symbol(t_outlet *o,t_symbol *s) { outlet_symbol(o,s); }
	V to_out_symbol(I n,t_symbol *s) { t_outlet *o = get_out(n); if(o) to_out_symbol(o,s); }
	V to_out_list(t_outlet *o,I argc,t_atom *argv) { outlet_list(o,gensym("list"),argc,argv); }
	V to_out_list(I n,I argc,t_atom *argv)  { t_outlet *o = get_out(n); if(o) to_out_list(o,argc,argv); }
	V to_out_anything(t_outlet *o,t_symbol *s,I argc,t_atom *argv) { outlet_anything(o,s,argc,argv); }
	V to_out_anything(I n,t_symbol *s,I argc,t_atom *argv)  { t_outlet *o = get_out(n); if(o) to_out_anything(o,s,argc,argv); }
		
// --- argument list stuff ----------------------------------------
		
		
// --- list creation stuff ----------------------------------------


// xxx internal stuff xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

protected:

	flext_base();
	virtual ~flext_base();
		
	struct xlet {	
		enum type {
			tp_none = 0,tp_def,tp_float,tp_flint,tp_sym,tp_list,tp_sig,tp_any
		};

		xlet(type t): tp(t),nxt(NULL) {}
		~xlet();
		
		type tp;
		xlet *nxt;
	};
	
	V AddInlet(xlet::type tp,I mult) { AddXlet(tp,mult,inlist); }
	V AddOutlet(xlet::type tp,I mult) { AddXlet(tp,mult,outlist); }

#ifdef PROXYIN
	// this is temporary!!
	virtual V m_anything_n(I inlet,t_symbol *s,I argc,t_atom *argv);
#endif

private:

	xlet *inlist,*outlist;
	I incnt,outcnt,insigs,outsigs;
	t_outlet **outlets;

	V AddXlet(xlet::type tp,I mult,xlet *&root);	

#ifdef PROXYIN
#ifdef PD
	// proxy object (for additional inlets) stuff
	struct px_object;
	friend struct px_object;
#elif defined(MAXMSP)
	typedef object px_object;
	static V cb_px_anything(V *c,t_symbol *s,I argc,t_atom *argv);
	static V cb_px_int(V *c,I v);
	static V cb_px_float(V *c,F f);
	static V cb_px_bang(V *c);
#endif

	px_object **inlets;
#endif

	// callback functions

	static V cb_help(V *c);

	static V cb_loadbang(V *c);
#ifdef MAXMSP
	static V cb_assist(V *c,V *b,L msg,L arg,C *s);
#endif
};



// === flext_dsp ==================================================

// dsp enabled base object

class flext_dsp:
	public flext_base
{
	FLEXT_HEADER(flext_dsp,flext_base)
	
public:

	// returns current sample rate
	F samplerate() const { return srate; }
	

// --- inheritable virtual methods --------------------------------

	// called on every dsp init (don't expect valid data in the signal vectors!)
	//   n: frames (aka samples) in one signal vector
	//   insigs: array of input vectors  (get number with function cnt_insig())
	//   outsigs: array of output vectors  (get number with function cnt_outsig())
	virtual V m_dsp(I n,F *const *insigs,F *const *outsigs);

	// called with every signal vector (here you do the dsp calculation)
	//   n: frames (aka samples) in one signal vector
	//   insigs: array of input vectors  (get number with function cnt_insig())
	//   outsigs: array of output vectors  (get number with function cnt_outsig())
	virtual V m_signal(I n,F *const *insigs,F *const *outsigs) = 0;

	// called with "enable" message: pauses/resumes dsp
	virtual V m_dspon(BL on);
	

// --- inlet/outlet stuff -----------------------------------------	

	// add signal inlet
	V add_in_signal(I m = 1) { AddInlet(xlet::tp_sig,m); }

	// add signal outlet
	V add_out_signal(I m = 1) { AddOutlet(xlet::tp_sig,m); }


protected:
	
	flext_dsp();
	~flext_dsp();
	
private:

	F srate;
	BL dspon;


	// callback functions

	static V cb_dsp(V *c,t_signal **s);
	static V cb_dspon(V *c,FI on);

	// dsp stuff

	static t_int *dspmeth(t_int *w); 
	F **invecs,**outvecs;
};


#endif

