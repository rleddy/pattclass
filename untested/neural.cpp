
#include <iostream>


double dtanh(double w) {
	return(1-w*w);
}



double (*derivitive_default_sigmoid)(double) = dtanh;
double (*default_sigmoid)(double) = tanh;



class NeuralNode;


class Ply {

	public:

		Ply(unsigned int N,Ply *p) {
			_layer.size_limit(N,p);  /// initialize all nodes.
		}

		virtual ~Ply() {
		}


		vector<NeuralNode *>	*get_layer(void) {
									return(&(this->_layer));
								}


		void					feed_forward();
		void					feed_backward(double scaled_learning_factor);
		void					init_layer(int N,Ply *p);


		void					init_layer(int N,Ply *p)  {
									for ( int i = 0; i < N; i++ ) {
										_layer[i] = new NeuralNode(p);
									}
								}


		string					_symbol;

	protected:

		vector<NeuralNode *>	_layer;

};



class NeuralNode {

	public:

		NeuralNode() {
			_error = 0;
			_delta = 0;
			_activation = random(-1,1);

			_prev_ply = NULL;

			deriv_sigmoid = derivitive_default_sigmoid;
			sigmoid = default_sigmoid;

		}

		virtual ~NeuralNode() {

		}

	public:

		void						set_prev_ply(Ply *ply) {
										_prev_ply = ply;
									}

		double						activation(void) {  return(_activation); }
		double						delta(void) {  return(_delta); }


		void 						calc_delta(void) {

										_error = 0.0;
										map<NeuralNode *, double>::iterator mit;
										for ( mit = _forward_links.begin(); mit != _forward_links.end(); mit++ ) {
											NeuralNode *nn = mit->first;
											double weight = mit->second;
											_error += nn->delta()*weight;
										}

										_delta = _error * derivative_sigmoid(_activation);

									}


		void						change_weights(double scaled_learning_factor) {
										if ( _prev_ply ) {

											vector<NeuralNode *> *layer_nodes = _prev_ply->get_layer();
											vector<NeuralNode *>::iterator vit;

											for ( vit  = layer_nodes->begin(); vit != layer_nodes->end(); vit++ ) {
												NeuralNode *nn  = *vit;
												double weight = nn->_forward_links[this];

												double change = (this->_delta)*(nn->_activation);
												weight += scaled_learning_factor*change;

												nn->_forward_links[this] = weight;
											}

										}
									}


		double						derivative_sigmoid(double w) {
										return( (*deriv_sigmoid)(w) );
									}


		void						activate(void) {
										_activation = 0.0;
										if ( _prev_ply ) {

											vector<NeuralNode *> *layer_nodes = _prev_ply->get_layer();
											vector<NeuralNode *>::iterator vit;

											double sum = 0.0;

											for ( vit  = layer_nodes->begin(); vit != layer_nodes->end(); vit++ ) {
												NeuralNode *nn  = *vit;

												double weight = nn->_forward_links[this];
												sum += weight*(nn->_activation);
											}

											_activation = (* sigmoid)(sum);

										}
									}


	protected:

		Ply								*_prev_ply;

		map<NeuralNode *, double>		_forward_links;
		double							_error;
		double							_delta;
		double							_activation;


		double							(* sigmoid)(double);
		double							(* deriv_sigmoid)(double);


};





class OutputNode {

	public:

		OutputNode() {
			_target = 0.0;
		}

		virtual ~OutputNode() {

		}

	public:

		void set_target(double target) {
			_target = target;
			_error = target - _activation;
			_delta = derivative_sigmoid(_activation)*_error;
		}

		double target(void) { return(_target); }



	protected:

		double					_target;

};



class InputNode {

	public:

		InputNode() {
			_symbol = "";
		}

		virtual ~InputNode() {

		}


	public:

		void	activate(void) {
			_activation = 1.0;
		}

		void	deactivate(void) {
			_activation = 0.0;
		}

};



class InputLayer : virtual public Ply {

	public:

		InputLayer(unsigned int N,Ply *p) {
			init_layer(N);
		}

		virtual ~InputLayer() {
		}


		void init_layer(int N)  {
									for ( int i = 0; i < N; i++ ) {
										_layer[i] = new InputNode();
									}
								}


};





class OutputLayer : virtual public Ply {

	public:

		OutputLayer(unsigned int N,Ply *p) {
			init_layer(N,p);
		}

		virtual ~OutputLayer() {
		}

		void init_layer(int N,Ply *p)  {
									for ( int i = 0; i < N; i++ ) {
										_layer[i] = new OutputNode(p);
									}
								}

		void set_targets(vector<double> &targets) {
									for ( int i = 0; i < _layer.size(); i++ ) {
										OutputNode *on = (OutputNode *)(_layer[i]);
										on->set_target(targets[i]);
									}
								}


};






void
Ply::init_layer(int N,Ply *p)  {
	for ( int i = 0; i < N; i++ ) {
		_layer[i] = new NeuralNode(p);
	}
}


void
Ply::feed_forward(void) {
	for ( int i = 0; i < _layer->size() ; i++ ) {
		_layer[i]->activate();
	}
}


void
Ply::feed_backward(double scaled_learning_factor) {

	for ( int i = 0; i < _layer->size() ; i++ ) {
		_layer[i]->calc_delta();
		_layer[i]->change_weights(scaled_learning_factor);
	}

}



list<Ply *> 				sg_layers;

map<string,InputNode *> 	sg_input_layer;


void
build_network(unsigned int N) {

	Ply *prev_ply = new InputLayer(N);

	sg_layers.push_back(new InputLayer(N));

	for ( unsigned int i = 1; i < N-1; i++ ) {
		Ply *p = new Ply(N,prev_ply)
		sg_layers.push_back(p);
		prev_ply = p;
	}

	sg_layers.push_back(new OutputLayer(N,prev_ply));

}




void
symbolize_input_layer(set<string> *vocabulary) {

	InputLayer *il = sg_layers->front();

	vector<NeuralNode *> *inputs = il->get_layer();
	vector<NeuralNode *>::iterator vit;

	vit = inputs->begin();

	set<string>::iterator sit;
	for ( sit = vocabulary->begin(); sit != vocabulary->end(); (sit++, vit++) ) {
		NeuralNode *nn = *vit;
		string word = *sit;

		nn->_symbol = word;

		sg_input_layer[word] = nn;
	}
}




void
symbolize_output_layer(set<string> *classification_labels) {

	InputLayer *il = sg_layers->back();

	vector<NeuralNode *> *outputs = il->get_layer();
	vector<NeuralNode *>::iterator vit;

	vit = outputs->begin();

	set<string>::iterator sit;
	for ( sit = classification_labels->begin(); sit != classification_labels->end(); (sit++, vit++) ) {
		NeuralNode *nn = *vit;
		string word = *sit;

		nn->_symbol = word;
	}

}




void
forward_propogation(set<string> *words) {

	list<Ply *>::iterator lit = sg_layers->begin();

	map<string,InputNode *>::iterator mit;
	mit = sg_input_layer.begin();
	while ( mit != sg_input_layer.end() ) {
		mit->second->deactivate();
		mit++;
	}


	for ( set<string>::iterator sit = words->begin(); sit != words->end(); sit++ ) {
		string word  = *sit;

		InputNode *nn = sg_input_layer[word];
		nn->activate();

	}
	lit++;



	while ( ++lit != sg_layers->end() ) {
		Ply *p = *lit;
		p->feed_forward();
	}


}



void
backward_propogation(vector<double> &targets,double scaled_learning_factor) {

	list<Ply *>::iterator lit = sg_layers->end();

	lit--;

	OutputLayer *ol = *lit;
	ol->set_targets(targets);

	lit--;

	do {

		Ply *p = *lit;
		p->feed_backward(scaled_learning_factor);

	} while ( *lit != sg_layers->begin() );

}



void
train(list<set<string> *>   *word_sets, list< vector<double> *> *targets,double scaled_learning_factor) {

	list<set<string> *>::iterator lit;
	list< vector<double> >::iterator vlit;

	for ( lit = word_sets->begin(), vlit = targets->begin() ; lit != word_sets->end(); (lit++, vlit++) ) {
		set<string> *trainers = *list;
		vector<double> *target = *vlit;

		forward_propogation(trainers);
		backward_propogation(*target,scaled_learning_factor);
	}

}



int
main(int argc, char **argv) {

	list<set<string> *>   *word_sets = load_words();
	list< vector<double> *> *targets = load_targets();

	train(word_sets,targets,0.5);


	/// get cases and report....

	return(0);
}








