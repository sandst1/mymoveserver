#include <fann.h>
#include <stdio.h>

int main()
{
    const unsigned int num_input = 500;
    const unsigned int num_output = 26;
    const unsigned int num_layers = 4;
    const unsigned int num_neurons_hidden = 4;
    const float desired_error = (const float) 0.001;
    const unsigned int max_epochs = 200000;
    const unsigned int epochs_between_reports = 1000;
    printf("Creating the network struct..\n");
    struct fann *ann = fann_create_standard(num_layers,
                                            num_input,
                                            num_neurons_hidden,
                                            num_output);

//    fann_set_learning_rate(ann, 0.8);
    fann_set_activation_function_hidden(ann, FANN_SIGMOID_STEPWISE);
    fann_set_activation_function_output(ann, FANN_LINEAR_PIECE);
    fann_set_training_algorithm(ann, FANN_TRAIN_INCREMENTAL);
    fann_set_train_stop_function(ann, FANN_STOPFUNC_MSE);
    printf("Training the network...\n");
    fann_train_on_file(ann, 
                       "ann_500", 
                       max_epochs, 
                       epochs_between_reports,
                       desired_error);
    printf("Training done. Saving to file...\n");
    fann_save(ann, "mymoves_gestures.net");
    fann_destroy(ann);
    printf("Done.\n");
    return 0;
}
