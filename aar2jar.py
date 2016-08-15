#!/usr/bin/python

# aar2jar.py (c) kak2 <doanngocbao@gmail.com>
# https://github.com/kak2/aar2jar

import sys, getopt, os
import shutil
import zipfile

mswindows = (sys.platform == "win32")
def main(argv):
   project_name = ''
   output_dir = ''
   if len(argv) == 0:
      help()
      sys.exit(2)
   try:
      opts, args = getopt.getopt(argv,"hi:o:",["iname=","odir="])
   except getopt.GetoptError:
      help()
      sys.exit(2)
   for opt, arg in opts:
      if opt == '-h':
         help()
         sys.exit()
      elif opt in ("-i", "--iname"):
         project_name = arg
      elif opt in ("-o", "--odir"):
         output_dir = arg
   
   convert(project_name, output_dir)

def help():
   print 'convert.py -i <your-project-name-without-aar> -o <output-dir>'

def convert(project_name, output_dir):
   if not existProject(project_name):
      print project_name, " is not existed in current directory"
      sys.exit(2)
   createOrCleanOutputDir(output_dir)
   
    # Convert your project first
   convert_project(project_name, output_dir)
   # Move all jar and convert other aar files
   files = os.listdir(os.curdir)
   for file in files:
      file_name, extension = os.path.splitext(file)
      print file_name, extension
      if extension == '.jar':
         shutil.move(file, os.path.join(output_dir, project_name, 'libs', file))
      elif extension == '.aar':
         if file_name != project_name:
            convert_project(file_name, output_dir)


def convert_project(project_name, output_dir):
   # Extract your project's file
   your_file_dest_path = os.path.join(output_dir, project_name)
   os.makedirs(your_file_dest_path)
   with zipfile.ZipFile(project_name + '.aar' , "r") as z:
      z.extractall(your_file_dest_path)
   #
   # Make it into ant library project
   #
   if not os.path.exists(os.path.join(your_file_dest_path, 'src')):
      os.makedirs(os.path.join(your_file_dest_path, 'src')) # Make src directory in project, but leave it empty
   if not os.path.exists(os.path.join(your_file_dest_path, 'libs')):
      os.makedirs(os.path.join(your_file_dest_path, 'libs')) # Make lib directory in project if it does not exists
   # Rename classes.jar to project_name.jar and move it into libs directory
   os.rename(os.path.join(your_file_dest_path, 'classes.jar'), os.path.join(your_file_dest_path, project_name + '.jar'))
   shutil.move(os.path.join(your_file_dest_path, project_name + '.jar'), os.path.join(your_file_dest_path, 'libs/' + project_name + '.jar'))
   # Call android update. So we have to install android first, then add it into PATH
   command = "android update lib-project -p " + os.path.join(output_dir, project_name) + " -t 10"
   os.system(command)
   # Update project.properties file
   with open(os.path.join(output_dir, project_name, 'project.properties'), "a") as myfile:
      myfile.write("android.library=true\n")
   
def createOrCleanOutputDir(output_dir):
   files = os.listdir(os.curdir)
   if os.path.exists(output_dir):
      deleteDir(output_dir)
   os.makedirs(output_dir)

def remove_readonly(fn, path, excinfo):
    #removes readonly tag from files/folders so they can be deleted
    if fn is os.rmdir:
        os.chmod(path, stat.S_IWRITE)
        os.rmdir(path)
    elif fn is os.remove:
        os.chmod(path, stat.S_IWRITE)
        os.remove(path)

def deleteDir(directory):
   shutil.rmtree(directory, onerror=remove_readonly)

def existProject(project_name):
   project_file = project_name + ".aar"
   files = os.listdir(os.curdir)
   try:
      files.index(project_file)
      return True
   except ValueError:
      return False

if __name__ == "__main__":
   main(sys.argv[1:])