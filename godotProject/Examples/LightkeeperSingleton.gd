@tool
extends Node

# This keeps our list of light RIDs
var active_light_rids: Array[Light3D] = []

func _enter_tree() -> void:
	# Connect to the tree to catch any new lights
	if not get_tree().node_added.is_connected(_on_node_added):
		get_tree().node_added.connect(_on_node_added)
	
	# Initialize existing lights (useful when you first open the scene)
	_refresh_lights()

func _on_node_added(node: Node) -> void:
	if node is Light3D:
		_register(node)

func _refresh_lights() -> void:
	active_light_rids.clear()
	# Note: in Tool mode, you might want to find children of the edited scene root
	var lights = get_tree().get_nodes_in_group("lights") # Or use a manual search
	for light in lights:
		if light is Light3D:
			_register(light)

func _register(light: Light3D) -> void:
	if not active_light_rids.has(light):
		active_light_rids.append(light)
		# Handle cleanup
		if not light.tree_exited.is_connected(_on_light_exited):
			light.tree_exited.connect(_on_light_exited.bind(light))

func _on_light_exited(node: Light3D) -> void:
	active_light_rids.erase(node)

# This is what your C++ code calls
func get_all() -> Array[Light3D]:
	return active_light_rids
