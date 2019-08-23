#include "KMemDriver.h"
#include "Imports.h"
#include "Native.h"

#include <ntddk.h>
#include <Ntstrsafe.h>

TABLE_SEARCH_RESULT
VADFindNodeOrParent(
	IN PMM_AVL_TABLE Table,
	IN ULONG_PTR StartingVpn,
	OUT PMMADDRESS_NODE *NodeOrParent
)
{
	PMMADDRESS_NODE Child;
	PMMADDRESS_NODE NodeToExamine;
	PMMVAD_SHORT    VpnCompare;
	ULONG_PTR       startVpn;
	ULONG_PTR       endVpn;

	if (Table->NumberGenericTableElements == 0) {
		return TableEmptyTree;
	}

	NodeToExamine = (PMMADDRESS_NODE)GET_VAD_ROOT(Table);

	for (;;) {

		VpnCompare = (PMMVAD_SHORT)NodeToExamine;
		startVpn = VpnCompare->StartingVpn;
		endVpn = VpnCompare->EndingVpn;

		startVpn |= (ULONG_PTR)VpnCompare->StartingVpnHigh << 32;
		endVpn |= (ULONG_PTR)VpnCompare->EndingVpnHigh << 32;

		KDBG("Examining Node 0x%p with start VA 0x%p and end VA 0x%p\n", VpnCompare, startVpn, endVpn);

		//
		// Compare the buffer with the key in the tree element.
		//

		if (StartingVpn < startVpn) {

			Child = NodeToExamine->LeftChild;

			if (Child != NULL) {
				NodeToExamine = Child;
			}
			else {

				//
				// Node is not in the tree.  Set the output
				// parameter to point to what would be its
				// parent and return which child it would be.
				//

				*NodeOrParent = NodeToExamine;
				return TableInsertAsLeft;
			}
		}
		else if (StartingVpn <= endVpn) {

			//
			// This is the node.
			//

			*NodeOrParent = NodeToExamine;
			return TableFoundNode;
		}
		else {

			Child = NodeToExamine->RightChild;

			if (Child != NULL) {
				NodeToExamine = Child;
			}
			else {

				//
				// Node is not in the tree.  Set the output
				// parameter to point to what would be its
				// parent and return which child it would be.
				//

				*NodeOrParent = NodeToExamine;
				return TableInsertAsRight;
			}
		}
	}
}

NTSTATUS VADFind(
	IN PEPROCESS pProcess,
	IN ULONG_PTR address,
	OUT PMMVAD_SHORT* pResult
)
{
	NTSTATUS status = STATUS_SUCCESS;
	ULONG_PTR vpnStart = address >> PAGE_SHIFT;
	PMM_AVL_TABLE pTable = (PMM_AVL_TABLE)((PUCHAR)pProcess + VAD_TREE_1803);
	PMM_AVL_NODE pNode = GET_VAD_ROOT(pTable);

	if (pProcess == NULL || pResult == NULL)
		return STATUS_INVALID_PARAMETER;

	// Search VAD
	if (VADFindNodeOrParent(pTable, vpnStart, &pNode) == TableFoundNode)
	{
		*pResult = (PMMVAD_SHORT)pNode;
	}
	else
	{
		KDBG("%s: VAD entry for address 0x%p not found\n", __FUNCTION__, address);
		status = STATUS_NOT_FOUND;
	}

	return status;
}

NTSTATUS VADProtect(
	IN PEPROCESS pProcess,
	IN ULONG_PTR address, IN ULONG prot
)
{
	NTSTATUS status = STATUS_SUCCESS;
	PMMVAD_SHORT pVadShort = NULL;

	status = VADFind(pProcess, address, &pVadShort);
	if (NT_SUCCESS(status))
		pVadShort->u.VadFlags.Protection = prot;

	return status;
}