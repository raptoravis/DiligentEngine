
git fetch ups
git merge ups/master

rem git submodule update --recursive --remote
git submodule update --recursive

git push


cd DiligentCore
git fetch ups
git merge ups/master
git push

cd ..


echo done

pause


