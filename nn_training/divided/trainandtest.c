#include <stdio.h>

#include "fann.h"

int main()
{
	const unsigned int num_layers = 3;
	const unsigned int num_neurons_hidden = 50;
	const float desired_error = (const float) 0.001;
	const unsigned int max_epochs = 350;
	const unsigned int epochs_between_reports = 10;
	struct fann *ann;
	struct fann_train_data *train_data, *test_data;

	unsigned int i = 0;

	printf("Creating network.\n");

	train_data = fann_read_train_from_file("trainingdata");

	ann = fann_create_standard(num_layers,
					  train_data->num_input, num_neurons_hidden, train_data->num_output);

	printf("Training network.\n");

	fann_set_activation_function_hidden(ann, FANN_SIGMOID_SYMMETRIC_STEPWISE);
	fann_set_activation_function_output(ann, FANN_LINEAR_PIECE); //FANN_SIGMOID_STEPWISE);

    fann_set_training_algorithm(ann, FANN_TRAIN_INCREMENTAL);

	fann_train_on_data(ann, train_data, max_epochs, epochs_between_reports, desired_error);

	printf("Testing network.\n");

	test_data = fann_read_train_from_file("testdata");

	fann_reset_MSE(ann);
	for(i = 0; i < fann_length_train_data(test_data); i++)
	{
		fann_test(ann, test_data->input[i], test_data->output[i]);
	}
	
	printf("MSE error on test data: %f\n", fann_get_MSE(ann));

	printf("Saving network.\n");

	fann_save(ann, "mymoves_gestures.net");

	printf("Cleaning up.\n");
	fann_destroy_train(train_data);
	fann_destroy_train(test_data);
	fann_destroy(ann);

	return 0;
}
