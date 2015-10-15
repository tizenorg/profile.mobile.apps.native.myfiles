#filename:feature_rm.py
#author:Liu Ruichao
import os
import shutil

feature_file = "CMakeLists.txt"
spec_file = "packaging/org.tizen.myfile.spec"
dest_path = os.getcwd()

old_str = "com.samsung"
new_str = "org.tizen"

def make_clear_line(line, prefix, profix):
	line_clear_blank = line.strip()
	line_rm_profix = line_clear_blank.strip(profix)
	line_rm_prefix = line_rm_profix.strip(prefix)
	line_clear = line_rm_prefix.strip()
	return line_clear

def read_feature_name(filepath):
	global feature_file, dest_path
	file_list_name = dest_path +os.sep+filepath
	data = open(file_list_name)

	feature_set = set()
	start = 0
	for each_line in data:
		line_clear = make_clear_line(each_line, '#', '')

		if start == 0:
			if line_clear == "START_PUBLIC_DISABLE_FEATURE":
				start = 1
			else:
				continue
		else:
			if line_clear == "END_START_PUBLIC_DISABLE_FEATURE":
				start = 0
			else:
				if len(line_clear) != 0:
					feature_set.add(line_clear)

	data.close()
	return feature_set

def read_file_name(filepath):
	global feature_file, dest_path
	file_list_name = dest_path +os.sep+filepath
	data = open(file_list_name)

	file_set = set()
	start = 0
	for each_line in data:
		line_clear = make_clear_line(each_line, '#', '')

		if start == 0:
			if line_clear == "START_PUBLIC_REMOVED_FILE":
				start = 1
			else:
				continue
		else:
			if line_clear == "END_START_PUBLIC_REMOVED_FILE":
				start = 0
			else:
				if len(line_clear) != 0:
					file_set.add(line_clear)

	data.close()
	return file_set

def read_string_removal(filepath):
	global feature_file, dest_path, spec_file
	file_name = dest_path +os.sep+filepath

	string_set = set()
	start = 0

	try:
		data = open(file_name)
		for each_line in data:
			line_clear = make_clear_line(each_line, '#', '')

			if start == 0:
				if line_clear == "START_PUBLIC_REMOVED_STRING":
					start = 1
				else:
					continue
			else:
				if line_clear == "END_START_PUBLIC_REMOVED_STRING":
					start = 0
				else:
					if len(line_clear) != 0:
						string_set.add(line_clear)
	except:
		print "open" + file_name + "failed"
	finally:
		data.close()

	file_name = dest_path + os.sep + spec_file
	try:
		print(file_name)
		data = open(file_name)
		for each_line in data:
			line_clear = make_clear_line(each_line, '#', '')

			if start == 0:
				if line_clear == "START_PUBLIC_REMOVED_STRING":
					start = 1
				else:
					continue
			else:
				if line_clear == "END_START_PUBLIC_REMOVED_STRING":
					start = 0
				else:
					if len(line_clear) != 0:
						string_set.add(line_clear)
	except:
		print "spec file does not exists"
	finally:
		data.close()

	return string_set

def replace_string(original, old, new):
	modified = original
	if original.find(old) != -1:
		modified = original.replace(old, new)
	return modified

def remove_code(filename, feature_set, string_set):
	global old_str, new_str
	start_remove = 0;
	taglist = list();
	dirty_taglist = list();
	# rename the original file
	old_filename = filename + ".tmp"
	new_filename = filename

	#move old file to be .tmp and create new file with original filename
	shutil.move(filename, old_filename)
	# open both file
	file_new = open(new_filename, "w")
	file_old = open(old_filename, "r")

	for each_line in file_old:
		#clear the line
		clear_line = make_clear_line(each_line, '', '')
		if start_remove == 0:	#it's not in a #ifdef that need to be removed
			if clear_line.startswith("#ifdef"):
				line_flag = make_clear_line(clear_line, "#ifdef", '')
				#print(line_flag)
				if line_flag in feature_set:
					taglist.append('d')
					start_remove = 1
				else:
					taglist.append('c')
					file_new.write(each_line)
			elif clear_line.startswith("#if"):
				taglist.append('c')
				file_new.write(each_line)
			elif clear_line.startswith("#endif"):
				if len(taglist) > 0:
					top = taglist.pop()
					if top == 'c':
						file_new.write(each_line)
					elif top == 'd':
						continue
			else:
				string_del_flag = False
                                for item in string_set:
                                        if clear_line == item:
                                                string_del_flag = True
                                                break
                                if string_del_flag == False:
                                	if clear_line.startswith("#START_PUBLIC_") or clear_line.startswith("#END_START_PUBLIC_"):
                                		continue;
                                	else:
                                        	file_new.write(each_line)

		else:	#it's in a #ifdef that need to be removed
			if clear_line.startswith("#ifdef"):
				taglist.append('d')
				dirty_taglist.append('d')
			elif clear_line.startswith("#else"):
				if len(dirty_taglist) == 0:
					start_remove = 0
			elif clear_line.startswith("#if") and not clear_line.startswith("#ifdef"):
				taglist.append('d')
				dirty_taglist.append('d')
			elif clear_line.startswith("#endif"):
				if len(dirty_taglist) > 0:
					dirty_taglist.pop()
				else:
					start_remove = 0
				taglist.pop()
			else:
				continue

	if len(taglist) > 0:
		print "\033[1;31;47m [Critical error] ",len(taglist)," tag(s) not matched! \033[0m"
	file_old.close()
	file_new.close()
	#at the end we should remove the .tmp file
	os.remove(old_filename)

def rename_operation(filename):
	global old_str, new_str
	# rename the original file
	old_filename = filename + ".tmp"
	new_filename = filename

	#move old file to be .tmp and create new file with original filename
	shutil.move(filename, old_filename)
	# open both file
	file_new = open(new_filename, "w")
	file_old = open(old_filename, "r")

	for each_line in file_old:
		#clear the line
		new_line = replace_string(each_line, old_str, new_str)
		file_new.write(new_line)

	file_old.close()
	file_new.close()
	#at the end we should remove the .tmp file
	os.remove(old_filename)

feature_set = read_feature_name(feature_file)
file_set = read_file_name(feature_file)
string_set = read_string_removal(feature_file)
print("feature list:")
for item in feature_set:
	print(item)

print("file list:")
for item in file_set:
	print(item)

print("string list:")
for item in string_set:
	print(item)

#remove_code("/home/rico/codebox/obs/music-player/src/view/mp-play-view.c", feature_set)


files=os.walk(dest_path)

for a,b,c in files:
	#we do not modify hidden directory
	check_ignore = a
	hidden_flag = False
	path_depth = check_ignore.split('/')
	for item in path_depth:
		if item.startswith("."):
			hidden_flag = True
			break

	if hidden_flag == True:
		continue

	#delete directory to be removed
	if len(b) != 0:
		for original_dir in b:
			if original_dir.startswith("."):
				continue

			dir_for_checking = a + os.sep + original_dir
			#check if directory to be removed
			remove_dir = False
			for item in file_set:
				clear_item = make_clear_line(item, '', '/')
				if dir_for_checking.find(clear_item) >= 0:
					remove_dir = True
					break
			if remove_dir == True:
				shutil.rmtree(dir_for_checking)
				continue

	if len(c)!=0:
		#check all files
		for original_file in c:
			#we do not modify hidden files
			if original_file.startswith("."):
				continue

			file_in_checking = a + os.sep + original_file
			#remove the specialized file
			delete_flag = False
			for item in file_set:
				if file_in_checking.find(item) >= 0:
					os.remove(file_in_checking)
					delete_flag = True
					break
			if delete_flag == True:
				continue

			remove_code(file_in_checking, feature_set, string_set)
			print "\033[1;34;47m [info hint]\033[0m" + file_in_checking + "\033[1;34;47m done\033[0m"

print "\033[1;35;48m [info hint]\033[0m" +"replace " + old_str + " with " + new_str  + "\033[1;34;47m done\033[0m"
files=os.walk(dest_path)
for a,b,c in files:
	#we do not modify hidden directory
	check_ignore = a
	hidden_flag = False
	path_depth = check_ignore.split('/')
	for item in path_depth:
		if item.startswith("."):
			hidden_flag = True
			break

	if hidden_flag == True:
		continue

	#start to replace org.tizen with org.tizen#
	## firstly, do file rename and content update##
	if len(c) != 0:
		for file_item in c:
			if file_item.startswith("."):
				continue

			file_in_checking = a + os.sep + file_item
			new_file_to_check = file_in_checking
			#check if file need to be rename
			new_file_name = replace_string(file_item, old_str, new_str)
			if new_file_name != file_item:
				new_file_to_check = a + os.sep + new_file_name
				shutil.move(file_in_checking, new_file_to_check)
			## do content update
			rename_operation(new_file_to_check)

	if len(b) != 0:
		for original_dir in b:
			dir_in_checking = a + os.sep + original_dir
			new_dir_to_check = dir_in_checking
			#check if dir need to be rename
			new_dir_name = replace_string(original_dir, old_str, new_str)
			if new_dir_name != original_dir:
				new_dir_to_check = a + os.sep + new_dir_name
				shutil.move(dir_in_checking, new_dir_to_check)

print "\033[1;34;47m [info hint]	dir/file content modification done! \033[0m"
