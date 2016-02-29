#ifndef MANTID_VATES_PRESENTER_FACTORIES_H
#define MANTID_VATES_PRESENTER_FACTORIES_H

#include "MantidAPI/IMDWorkspace.h"
#include "MantidKernel/make_unique.h"

namespace Mantid {
namespace VATES {

// Forward Decalaration
class MDLoadingView;
class WorkspaceProvider;

class DLLExport EmptyWorkspaceNamePolicy {
protected:
  std::string getWorkspaceName(Mantid::API::IMDWorkspace_sptr) {
    return "__EmptyWorkspaceNamePolicy";
  }
};

class DLLExport NonEmptyWorkspaceNamePolicy {
protected:
  std::string getWorkspaceName(Mantid::API::IMDWorkspace_sptr workspace) {
    return workspace->name();
  }
};

/**
* This templated function sets up an in memory loading presenter.
* @param view: the loading view type
* @param wsName: the name of the workspace which is to be displayed
* @param worksapceProvider: a worksapce provider
* @returns a new in memory loading presenter.
*/
template <class Presenter, class WorkspaceNamePolicy>
class DLLExport InMemoryPresenterFactory : private WorkspaceNamePolicy {
  using WorkspaceNamePolicy::getWorkspaceName;

public:
  std::unique_ptr<Presenter>
  create(std::unique_ptr<MDLoadingView> view,
         Mantid::API::IMDWorkspace_sptr workspace,
         std::unique_ptr<WorkspaceProvider> workspaceProvider) {
    return Mantid::Kernel::make_unique<Presenter>(std::move(view),
                                                  workspaceProvider.release(),
                                                  getWorkspaceName(workspace));
  }
};

}
}
#endif