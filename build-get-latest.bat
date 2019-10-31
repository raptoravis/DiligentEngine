
echo get the latest core...

git pull
git submodule update --recursive

echo get the latest core(ups)...

cd DiligentCore
git fetch ups
git merge ups/master
git push

cd ..

echo get the latest(ups) ...

git fetch ups
git merge ups/master

rem git submodule update --recursive --remote
git submodule update --recursive

rem git push


echo check and to push...

pause


