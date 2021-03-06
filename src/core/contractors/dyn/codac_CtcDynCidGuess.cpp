/*
 *  CtcDynCidGuess class
 * ----------------------------------------------------------------------------
 * 	\date       2020
 *  \authors  	Victor Reyes
 */
#include "codac_CtcDynCidGuess.h"
#include <deque>

using namespace std;
using namespace ibex;



namespace codac
{
  CtcDynCidGuess::CtcDynCidGuess(const TFnc& fnc, double prec): fnc(fnc), prec(prec)
	{
		//assert(prec >= 0);
		set_prec(0.05);
	}

	bool CtcDynCidGuess::contract(std::vector<Slice*> x_slice, std::vector<Slice*> v_slice, TimePropag t_propa)
	{
		//checks that the domain of each slice is the same.
		Interval to_try(x_slice[0]->tdomain());
		for (int i = 1 ; i < x_slice.size(); i++)
			//assert(to_try == x_slice[i]->tdomain());

		//check if the gates used to contract are bounded
		for (int i = 0 ; i < x_slice.size(); i++){
			if ((t_propa & TimePropag::FORWARD) && (x_slice[0]->input_gate().is_unbounded()))
				return false;
			else if ((t_propa & TimePropag::BACKWARD) && (x_slice[0]->output_gate().is_unbounded()))
				return false;
		}
		//polygone method setter
		if (m_fast_mode)
			ctc_deriv.set_fast_mode(true);
		bool first_iteration = true;
		bool fix_point2;
		do{
			fix_point2=false;
			double volumex_1 = 0; double volumex_2 = 0;
			for (int i = 0; i < x_slice.size() ; i++)
				volumex_1 = volumex_1 + x_slice[i]->volume();
			vector<Slice> x_slice_bounds;
			vector<Slice> v_slice_bounds;
			x_slice_bounds.clear(); v_slice_bounds.clear();

			for (int i = 0 ; i < x_slice.size() ; i++){
				x_slice_bounds.push_back(*x_slice[i]);
				v_slice_bounds.push_back(*v_slice[i]);
			}

			//save volume before contraction
			double volume_old = 0;
			for (int i = 0; i < x_slice.size() ; i++)
				volume_old = volume_old + x_slice[i]->volume();

			/*propagation engine*/
			bool fix_point;
			int max_iterations;

			if (get_propagation_engine() == 0){
				AtomicPropagationEngine(x_slice_bounds,v_slice_bounds,t_propa);
			}
			else if (get_propagation_engine() == 1){
				FullPropagationEngine(x_slice_bounds,v_slice_bounds,t_propa);
			}

			/*3B part*/
			for (int i = 0 ; i < x_slice.size() ; i++){
				double aux_envelope = x_slice[i]->codomain().diam();
				Interval remove_ub,remove_lb;
				if (t_propa & TimePropag::FORWARD){
					remove_ub = Interval(x_slice_bounds[i].output_gate().ub(),x_slice[i]->output_gate().ub());
					remove_lb = Interval(x_slice[i]->output_gate().lb(),x_slice_bounds[i].output_gate().lb());
				}
				else if (t_propa & TimePropag::BACKWARD){
					remove_ub = Interval(x_slice_bounds[i].input_gate().ub(),x_slice[i]->input_gate().ub());
					remove_lb = Interval(x_slice[i]->input_gate().lb(),x_slice_bounds[i].input_gate().lb());
				}

				/*Check of the guess*/
				if (remove_ub.diam()>0){
					var3Bcheck(remove_ub,ub,i,x_slice,v_slice,t_propa);
				}
				if (remove_lb.diam()>0){
					var3Bcheck(remove_lb,lb,i,x_slice,v_slice,t_propa);
				}
				/*to update domains*/
				ctc_deriv.contract(*x_slice[i], *v_slice[i],t_propa);
				ctc_fwd(*x_slice[i],*v_slice[i], x_slice, v_slice, i);
				ctc_deriv.contract(*x_slice[i], *v_slice[i],t_propa);
				/*empty test*/
				if (x_slice[i]->is_empty()) return false;
			}
			for (int i = 0; i < x_slice.size() ; i++)
				volumex_2 = volumex_2 + x_slice[i]->volume();
			if (1-(volumex_2/volumex_1) > get_prec()) fix_point2 = true;

			/*incrementality test*/
			if ((first_iteration) && (volumex_1 == volumex_2))
				return false;
			first_iteration = false;
			if (get_max_it()) fix_point2 = false;

		} while(fix_point2);
		return true;
	}


	void CtcDynCidGuess::ctc_fwd(Slice &x, Slice &v, std::vector<Slice*> x_slice, std::vector<Slice*> v_slice, int pos)
	{
		/*envelope*/
		IntervalVector envelope(x_slice.size()+1);
		envelope[0] = x.tdomain();
		for (int i = 0 ; i < x_slice.size() ; i++){
			if (i==pos)
				envelope[i+1] = x.codomain();
			else
				envelope[i+1] = x_slice[i]->codomain();
		}
			v.set_envelope(fnc.eval_vector(envelope)[pos]);
	}

	void CtcDynCidGuess::ctc_fwd(Slice &x, Slice &v, std::vector<Slice> x_slice, std::vector<Slice> v_slice, int pos)
	{
		/*envelope*/
		IntervalVector envelope(x_slice.size()+1);
		envelope[0] = x.tdomain();

		for (int i = 0 ; i < x_slice.size() ; i++){
			if (i==pos)
				envelope[i+1] = x.codomain();
			else
				envelope[i+1] = x_slice[i].codomain();
		}

		v.set_envelope(fnc.eval_vector(envelope)[pos]);
	}

	double CtcDynCidGuess::get_prec()
	{
		return this->prec;
	}

	bool CtcDynCidGuess::get_max_it()
	{
		return this->max_it;
	}

	int CtcDynCidGuess::get_propagation_engine(){
		return this->engine;
	}

	int CtcDynCidGuess::get_s_corn(){
		return this->s_strategy;
	}

	int CtcDynCidGuess::get_dpolicy(){
		return this->d_policy;
	}

	void CtcDynCidGuess::set_prec(double prec){
		this->prec = prec;
	}

	void CtcDynCidGuess::set_max_it(bool max_it){
		this->max_it = max_it;
	}

	void CtcDynCidGuess::set_propagation_engine(int engine){
		this->engine = engine;
	}

	void CtcDynCidGuess::set_s_corn(int s_strategy){
			this->s_strategy = s_strategy;
	}

	void CtcDynCidGuess::set_dpolicy(int d_policy){
				this->d_policy = d_policy;
	}
	void CtcDynCidGuess::set_variant(int variant){
		if (variant==0){
			this->set_propagation_engine(0);
			this->set_prec(0.05);
			this->set_s_corn(0);
			this->set_dpolicy(0);
		}
		else if (variant==1){
			this->set_propagation_engine(1);
			this->set_prec(0.05);
			this->set_s_corn(1);
			this->set_dpolicy(2);
		}
		else if (variant==2){
			this->set_propagation_engine(1);
			this->set_prec(0.025);
			this->set_s_corn(1);
			this->set_dpolicy(2);
		}
	}

	void CtcDynCidGuess::create_slices(Slice& x_slice, std::vector<Interval> & x_slices, TimePropag t_propa)
	{

		/*Varcid in the input gate*/
		if (t_propa & TimePropag::FORWARD){
			x_slices.push_back(Interval(x_slice.input_gate().lb()));
			x_slices.push_back(Interval(x_slice.input_gate().ub()));
		}

		/*Varcid in the output gate*/
		else if (t_propa & TimePropag::BACKWARD){
			x_slices.push_back(Interval(x_slice.output_gate().lb()));
			x_slices.push_back(Interval(x_slice.output_gate().ub()));
		}
	}

	std::vector<std::vector<double>> CtcDynCidGuess::cart_product (const std::vector<std::vector<double>>& v) {
		std::vector<std::vector<double>> s = {{}};
	    for (const auto& u : v) {
	        vector<vector<double>> r;
	        for (const auto& x : s) {
	            for (const auto y : u) {
	                r.push_back(x);
	                r.back().push_back(y);
	            }
	        }
	        s = move(r);
	    }
	    return s;
	}

	void CtcDynCidGuess::create_corners(std::vector<Slice> x_slices, std::vector< std::vector<double> > & points, TimePropag t_propa){

		std::vector< std::vector<double> > aux_points;
		//for each dimension, obtain the corresponding corners
		if (t_propa & TimePropag::FORWARD){
			for (int i=0 ; i < x_slices.size() ; i++){
				vector<double> aux;
				aux.clear();
				aux.push_back(x_slices[i].input_gate().lb());
				aux.push_back(x_slices[i].input_gate().ub());
				aux_points.push_back(aux);
			}
		}
		else if(t_propa & TimePropag::BACKWARD){
			for (int i=0 ; i < x_slices.size() ; i++){
				vector<double> aux;
				aux.clear();
				aux.push_back(x_slices[i].output_gate().lb());
				aux.push_back(x_slices[i].output_gate().ub());
				aux_points.push_back(aux);
			}
		}
		//create the 2n corners
		points = cart_product(aux_points);
	}

	void CtcDynCidGuess::var3Bcheck(ibex::Interval remove_bound ,int bound, int pos ,std::vector<Slice*> & x_slice, std::vector<Slice*> v_slice, TimePropag t_propa)
	{

		ctc_deriv.set_fast_mode(false);
		bool fix_point_n;
		vector<Slice> x_slice_bounds;
		vector<Slice> v_slice_bounds;
		for (int i = 0 ; i < x_slice.size() ; i++){
			x_slice_bounds.push_back(*x_slice[i]);
			v_slice_bounds.push_back(*v_slice[i]);
		}
		if (t_propa & TimePropag::FORWARD)
			x_slice_bounds[pos].set_output_gate(remove_bound);
		else if (t_propa & TimePropag::BACKWARD)
			x_slice_bounds[pos].set_input_gate(remove_bound);

		/*try to remove the complete interval*/
		for (int i = 0 ;  i < x_slice.size() ; i++){
			double sx;
			int max_iterations=0;
			do
			{
				sx = x_slice_bounds[i].volume();
				ctc_deriv.contract(x_slice_bounds[i], v_slice_bounds[i],t_propa);
				ctc_fwd(x_slice_bounds[i], v_slice_bounds[i], x_slice, v_slice, i);
				max_iterations++;
			} while((sx-x_slice_bounds[i].volume() > 0) && (max_iterations<50) );
//			if (max_iterations>=50) set_max_it(true);  // max iterations reached
			/*if something is empty means that we can remove the complete interval*/
			if (x_slice_bounds[i].is_empty()){
				if (t_propa & TimePropag::FORWARD){
					if (bound == ub)
						x_slice[pos]->set_output_gate(Interval(x_slice[pos]->output_gate().lb(),remove_bound.lb()));
					else if (bound == lb)
						x_slice[pos]->set_output_gate(Interval(remove_bound.ub(),x_slice[pos]->output_gate().ub()));
				}
				if (t_propa & TimePropag::BACKWARD){
					if (bound == ub)
						x_slice[pos]->set_input_gate(Interval(x_slice[pos]->input_gate().lb(),remove_bound.lb()));
					else if (bound == lb)
						x_slice[pos]->set_input_gate(Interval(remove_bound.ub(),x_slice[pos]->input_gate().ub()));
				}
				return;
			}
		}

		/*update the guess with x_slice_bounds*/
		if (t_propa & TimePropag::FORWARD){
			if (bound == ub)
				remove_bound = Interval(remove_bound.lb(),x_slice_bounds[pos].output_gate().ub());
			else if (bound == lb)
				remove_bound = Interval(x_slice_bounds[pos].output_gate().lb(),remove_bound.ub());
		}
		else if (t_propa & TimePropag::BACKWARD){
			if (bound == ub)
				remove_bound = Interval(remove_bound.lb(),x_slice_bounds[pos].input_gate().ub());
			else if (bound == lb)
				remove_bound = Interval(x_slice_bounds[pos].input_gate().lb(),remove_bound.ub());
		}

		/*dichotomic approach*/
		Interval subinterval_removal;  bool success;
		if (get_dpolicy() != 0){
			for (int k = 0 ;  k < 1 ; k++){
				double diam_removal = (remove_bound.diam()/1024)*(k+1);
				/*restore domains*/
				x_slice_bounds.clear(); v_slice_bounds.clear();
				for (int i = 0 ; i < x_slice.size() ; i++){
					x_slice_bounds.push_back(*x_slice[i]);
					v_slice_bounds.push_back(*v_slice[i]);
				}
				/*select policy*/

				if (bound == ub){
					if (get_dpolicy() == 1)
						subinterval_removal = Interval(remove_bound.mid(),remove_bound.ub());
					else if (get_dpolicy() == 2)
						subinterval_removal = Interval(remove_bound.ub()-diam_removal,remove_bound.ub());
				}
				else if (bound == lb){
					if (get_dpolicy() == 1)
						subinterval_removal = Interval(remove_bound.lb(),remove_bound.mid());
					else if (get_dpolicy() == 2)
						subinterval_removal = Interval(remove_bound.lb(),remove_bound.lb()+diam_removal);
				}
				if (t_propa & TimePropag::FORWARD)
					x_slice_bounds[pos].set_output_gate(subinterval_removal);
				else if (t_propa & TimePropag::BACKWARD)
					x_slice_bounds[pos].set_input_gate(subinterval_removal);
				success = false;
				for (int i = 0 ;  i < x_slice_bounds.size() ; i++){
					double sx;
					CtcDeriv ctc_deriv;
					int max_iterations=0;
	//				/*without polygons*/
					do
					{
						sx = x_slice_bounds[i].volume();
						ctc_deriv.contract(x_slice_bounds[i], v_slice_bounds[i],t_propa);
						ctc_fwd(x_slice_bounds[i], v_slice_bounds[i], x_slice, v_slice, i);
						max_iterations++;
					} while((sx-x_slice_bounds[i].volume() > 0) && (max_iterations<50) );
					/*if something is empty means that we can remove the half*/

					if (x_slice_bounds[i].is_empty()){
						success = true;
						if (bound == ub){
							if (get_dpolicy() == 1)
								remove_bound = Interval(remove_bound.lb(),remove_bound.mid());
							else if (get_dpolicy() == 2)
								remove_bound = Interval(remove_bound.lb(),remove_bound.ub()-diam_removal);
						}
						else if (bound == lb)
							if (get_dpolicy() == 1)
								remove_bound = Interval(remove_bound.mid(),remove_bound.ub());
							if (get_dpolicy() == 2)
								remove_bound = Interval(remove_bound.lb()+diam_removal,remove_bound.ub());
					}
					if (success)
						break;
				}
				if (!success){
					break;
				}

			}
		}

		/*update the bounds*/
		if (t_propa & TimePropag::FORWARD){
			if (bound==ub)
				x_slice[pos]->set_output_gate(Interval(x_slice[pos]->output_gate().lb(),remove_bound.ub()));
			else if (bound==lb)
				x_slice[pos]->set_output_gate(Interval(remove_bound.lb(),x_slice[pos]->output_gate().ub()));
		}
		else if (t_propa & TimePropag::BACKWARD){
			if (bound==ub)
				x_slice[pos]->set_input_gate(Interval(x_slice[pos]->input_gate().lb(),remove_bound.ub()));
			else if (bound==lb)
				x_slice[pos]->set_input_gate(Interval(remove_bound.lb(),x_slice[pos]->input_gate().ub()));
		}
	}

	void CtcDynCidGuess::AtomicPropagationEngine(std::vector<Slice> & x_slice, std::vector<Slice> & v_slice, TimePropag t_propa){


		for (int i = 0 ; i < x_slice.size() ;i++){

			std::vector<Interval> x_subslices;
			x_subslices.clear();

			create_slices(x_slice[i],x_subslices, t_propa);

			Interval hull_input_x = Interval::EMPTY_SET; Interval hull_input_v = Interval::EMPTY_SET;
			Interval hull_output_x = Interval::EMPTY_SET; Interval hull_output_v = Interval::EMPTY_SET;
			Interval hull_codomain_x = Interval::EMPTY_SET; Interval hull_codomain_v = Interval::EMPTY_SET;

			for (int j = 0 ; j < x_subslices.size() ; j++){

				/*Temporal slices on $x$ and $v$*/
				Slice aux_slice_x(x_slice[i]);
				Slice aux_slice_v(v_slice[i]);

				if (t_propa & TimePropag::FORWARD)
					aux_slice_x.set_input_gate(x_subslices[j]);
				else if (t_propa & TimePropag::BACKWARD)
					aux_slice_x.set_output_gate(x_subslices[j]);

				/*Fixpoint for each sub-slice at each tube*/
				double sx;
				int max_iterations=0;

				do
				{
					sx = aux_slice_x.volume();
					ctc_deriv.contract(aux_slice_x, aux_slice_v,t_propa);
					ctc_fwd(aux_slice_x, aux_slice_v, x_slice, v_slice, i);
					max_iterations++;
				} while((1-(aux_slice_x.volume()/sx)) > get_prec() && (max_iterations<50));

				if (max_iterations>=50) set_max_it(true);  // max iterations reached
				/*The union of the current guess is made.*/
				if (t_propa & TimePropag::BACKWARD){
					hull_input_x |= aux_slice_x.input_gate(); hull_input_v |= aux_slice_v.input_gate();
				}
				else if (t_propa & TimePropag::FORWARD){
					hull_output_x |= aux_slice_x.output_gate(); hull_output_v |=aux_slice_v.output_gate();
				}
				hull_codomain_x |= aux_slice_x.codomain(); hull_codomain_v |= aux_slice_v.codomain();
			}

			/*Intersection in all the dimensions*/
			x_slice[i].set_envelope(hull_codomain_x & x_slice[i].codomain() );  v_slice[i].set_envelope(hull_codomain_v & v_slice[i].codomain());
			if (t_propa & TimePropag::BACKWARD){
				x_slice[i].set_input_gate(hull_input_x & x_slice[i].input_gate()); v_slice[i].set_input_gate(hull_input_v & v_slice[i].input_gate());
			}
			else if (t_propa & TimePropag::FORWARD){
				x_slice[i].set_output_gate(hull_output_x & x_slice[i].output_gate()); v_slice[i].set_output_gate(hull_output_v & x_slice[i].output_gate());
			}
		}
	}

  void CtcDynCidGuess::FullPropagationEngine(std::vector<Slice> & x_slice, std::vector<Slice> & v_slice, TimePropag t_propa){

			/*create the contractor queue: format contraint - variable, 0: for ctc_deriv, 1 for fwd*/
			std::deque< vector<int> > contractorQ;

			vector<int> ctr_var;
			for (int i = 0 ; i < 2 ; i++) ctr_var.push_back(-1);

			/*Initialization contractor array - bool*/
			vector<bool > isPresent;

			for (int i = 0 ; i < x_slice.size() ; i++){
				isPresent.push_back(false);
			}

			std::vector< std::vector<double> >  points;
			if (get_s_corn() == 1)
				create_corners(x_slice, points, t_propa);

			/*going throw all the variables*/
			for (int i = 0 ; i < x_slice.size() ; i++){

				std::vector<ibex::Interval> x_subslices;
				x_subslices.clear();
				/*create the sub-slices*/
				if (get_s_corn() == 0)
					create_slices(x_slice[i],x_subslices, t_propa);

				/*Hull for each dimension on x and v*/
				vector<Interval> hull_input_x; vector<Interval> hull_input_v;
				vector<Interval> hull_output_x; vector<Interval> hull_output_v;
				vector<Interval> hull_codomain_x; vector<Interval> hull_codomain_v;

				/*Initiliazation*/
				for (int j = 0 ; j < x_slice.size() ; j++){
					hull_input_x.push_back(Interval::EMPTY_SET); hull_input_v.push_back(Interval::EMPTY_SET);
					hull_output_x.push_back(Interval::EMPTY_SET); hull_output_v.push_back(Interval::EMPTY_SET);
					hull_codomain_x.push_back(Interval::EMPTY_SET); hull_codomain_v.push_back(Interval::EMPTY_SET);
				}

				int nb_iterations;
				if (get_s_corn() == 0)
					nb_iterations = x_subslices.size();
				else if (get_s_corn() == 1)
					nb_iterations = points.size();
				for (int k = 0 ; k < nb_iterations ; k++){

					/*restore with the current domains*/
					vector<Slice> aux_slice_x; aux_slice_x.clear();
					vector<Slice> aux_slice_v; aux_slice_v.clear();

					for (int j = 0 ; j < x_slice.size() ; j++){
						aux_slice_x.push_back(x_slice[j]);
						aux_slice_v.push_back(v_slice[j]);
					}

					/*Set the gate depending on the direction of the contraction*/
					if (get_s_corn() == 0){
						if (t_propa & TimePropag::FORWARD)
							aux_slice_x[i].set_input_gate(x_subslices[k]);
						else if (t_propa & TimePropag::BACKWARD)
							aux_slice_x[i].set_output_gate(x_subslices[k]);
					}
					else if (get_s_corn() == 1){
						if (t_propa & TimePropag::FORWARD)
							for (int tt = 0 ; tt < points[k].size() ; tt++)
								aux_slice_x[tt].set_input_gate(points[k][tt]);

						else if (t_propa & TimePropag::BACKWARD)
							for (int tt = 0 ; tt < points[k].size() ; tt++)
								aux_slice_x[tt].set_output_gate(points[k][tt]);
					}

					/*push the first element to the contractor queue*/
					if (get_s_corn() == 0){
						ctr_var[0] = 0; ctr_var[1] = i;
					}
					else if (get_s_corn() == 1){
						ctr_var[0] = 0; ctr_var[1] = 0;
					}
					contractorQ.push_back(ctr_var);

					/*FIFO queue*/
					do{
						/*get what contractor should be called*/
						int contractor = contractorQ.front()[0];
						/*get the variable that is going to be contracted*/
						int variable = contractorQ.front()[1];
						isPresent[variable] = false;
						/*pop the first element*/
						contractorQ.pop_front();
						/*contract*/
						if (contractor == 0 ){ //call ctc_deriv
							/*save the corresponding domain*/
							double size_x = aux_slice_x[variable].codomain().diam();
							ctc_deriv.contract(aux_slice_x[variable],aux_slice_v[variable],t_propa);

							if ((1-(aux_slice_x[variable].codomain().diam()/size_x)) > this->get_prec()){
								ctr_var[0] = 1; ctr_var[1] = variable;
								contractorQ.push_front(ctr_var);
							}
						}
						else if (contractor == 1){ //call ctc_fwd
							/*save the corresponding domain*/
							double size_v = aux_slice_v[variable].codomain().diam();
							ctc_fwd(aux_slice_x[variable], aux_slice_v[variable], aux_slice_x, aux_slice_v, variable);

							/*add the contraints not included in isPresent*/
							if ((1-(aux_slice_v[variable].codomain().diam()/size_v)) > this->get_prec()){
								for (int j = 0 ; j < x_slice.size() ; j++ ){
									if ((!isPresent[j]) && (j!=variable)){
										ctr_var[0] = 1; ctr_var[1] = j;
										contractorQ.push_front(ctr_var);
										isPresent[j] = true;
									}
								}
								ctr_var[0] = 0; ctr_var[1] = variable;
								contractorQ.push_front(ctr_var);
							}
						}

					} while (contractorQ.size() > 0);

					/*The union of the current Slice is made*/
					for (int j = 0 ; j < x_slice.size() ; j++){
						hull_input_x[j] |= aux_slice_x[j].input_gate(); hull_input_v[j] |= aux_slice_v[j].input_gate();
						hull_output_x[j] |= aux_slice_x[j].output_gate(); hull_output_v[j] |= aux_slice_v[j].output_gate();
						hull_codomain_x[j] |= aux_slice_x[j].codomain(); hull_codomain_v[j] |= aux_slice_v[j].codomain();
					}
				}
				/*replacing the old domains*/
				for (int j = 0 ; j < x_slice.size() ; j++){
					x_slice[j].set_envelope(x_slice[j].codomain() & hull_codomain_x[j]); v_slice[j].set_envelope(v_slice[j].codomain() & hull_codomain_v[j]);
					x_slice[j].set_input_gate(x_slice[j].input_gate() & hull_input_x[j]); v_slice[j].set_input_gate(v_slice[j].input_gate() & hull_input_v[j]);
					x_slice[j].set_output_gate(x_slice[j].output_gate() & hull_output_x[j]); v_slice[j].set_output_gate(v_slice[j].output_gate() & hull_output_v[j]);
				}
				if (get_s_corn() == 1)
					i=x_slice.size()+1;
			}

		}
  
  void CtcDynCidGuess::contract(std::vector<Domain*>& v_domains) {;}  		  
  
}
