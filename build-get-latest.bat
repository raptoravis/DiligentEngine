cd DiligentCore
git fetch ups
git merge ups/master
git push

cd ..

git fetch ups
git merge ups/master

git submodule update --recursive --remote

git push

echo done

pause


