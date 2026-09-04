extends ExampleRunner

@onready var example_list_ui : OptionButton = $Control/VBoxContainer/ExampleList

var runner : ExampleRunner;
var example_names : Array[String];

func _ready() -> void:
	get_window().title = "Example Runner"
	runner = self
	runner.setRoot(self);
	example_names = runner.listExamples()
	for x in example_names:
		example_list_ui.add_item(x)
	
func _on_example_list_item_selected(_index: int) -> void:
	runner.runExample(_index, get_window().size);
