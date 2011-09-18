#include <stdio.h>

#include "fann.h"

int main()
{
	const float desired_error = (const float) 0.0001;
	const unsigned int max_epochs = 350;
	const unsigned int epochs_between_reports = 25;
	struct fann *ann;
	struct fann_train_data *train_data;

	unsigned int i = 0;

	printf("Creating network.\n");

	train_data = fann_read_train_from_file("ann_training_data");
    // Using incremental training -> shuffle training data    
    fann_shuffle_train_data(train_data);

//	ann = fann_create_standard(num_layers,
//					  train_data->num_input, num_neurons_hidden, train_data->num_output);

    //ann = fann_create_standard(500, 2, 50, 50, 21);
    unsigned int layers[4] = {150, 70, 30, 14};
    ann = fann_create_standard_array(4, layers);
	printf("Training network.\n");

	fann_set_activation_function_hidden(ann, FANN_SIGMOID_SYMMETRIC_STEPWISE);
	fann_set_activation_function_output(ann, FANN_SIGMOID);

    fann_set_training_algorithm(ann, FANN_TRAIN_INCREMENTAL);

	fann_train_on_data(ann, train_data, max_epochs, epochs_between_reports, desired_error);

	printf("Saving network.\n");

	fann_save(ann, "mymoves_nn2.net");

	printf("Cleaning up.\n");
	fann_destroy_train(train_data);
	fann_destroy(ann);

	return 0;
}
