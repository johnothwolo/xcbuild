//DBPluginSetName group
//DBPluginSetType basic
//
//proc usage {} {
//	return {<group>}
//}
//
//proc run {args} {
//	set group [lindex $args 0]
//	if {$group == ""} { return -1 }
//	foreach member [DBCopyGroupMembers [DBGetCurrentBuild] $group] {
//		puts $member
//	}
//}



#include "Plugin.h"

namespace plugin {

class Group : BasicPlugin<DBString> {
public:
    Group() : BasicPlugin("group")   {}

    // FIXME: finish this
    int run(DBDatabase &db, const std::string& build, const std::vector<std::string>& argv) override {
        std::string group = argv.at(0);
        if (argv.empty())
            return -1;
        
        auto members = db.copyGroupMembers<std::string>(build, group);
        
        for(auto &member : members)
            std::cerr << member << std::endl;
        
        return 0;
    }
    
    std::string usage() override { return "<group>"; }
};


//#pragma section("plugins",read,write,execute)
DECLARE_PLUGIN(Group, group);

} // namespace plugin
